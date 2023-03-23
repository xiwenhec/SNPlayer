//
// Created by sivin on 11/28/22.
//

#define LOG_TAG "CurlConnection"

#include "CurlConnection.h"

#include <utility>
#include <utils/SNLog.h>
#include <utils/SNTimer.h>
#include "data_source/curl/CurlConnectionManager.h"

#define MAX_HEADER_SIZE 1024
#define RINGBUFFER_SIZE (1024 * 256)
#define RINGBUFFER_BACK_SIZE (1024 * 512)

#define CURL_READ_BUFFER_SIZE CURL_MAX_WRITE_SIZE//(1024 * 16)

#define TRUE 1L
#define FALSE 0L

namespace Sivin {

  static int getErrorCode(const CURLcode &CURLResult) {
    //TODO:Sivin ERROR CODE;
    return -1;
  }

  CurlConnection::CurlConnection(IDataSource::SourceConfig *config,
                                 std::shared_ptr<CurlConnectionManager> connectionManager,
                                 std::shared_ptr<ConnectionListener> listener) :
          mConfig(config),
          mConnectionManager(std::move(connectionManager)),
          mListener(std::move(listener)),
          mBuffer(std::make_shared<RingBuffer>(RINGBUFFER_SIZE + RINGBUFFER_BACK_SIZE,
                                               RINGBUFFER_BACK_SIZE)) {
    isFirstLoop = true;
    mHttpHandle = curl_easy_init();
    if (mConfig != nullptr) {
      if (mConfig->lowSpeedLimit && mConfig->lowSpeedTimeMs) {
        SN_LOGD("set lowSpeedLimit to %d", mConfig->lowSpeedLimit);
        SN_LOGD("set lowSpeedTime to %d(ms)", mConfig->lowSpeedTimeMs);
        curl_easy_setopt(mHttpHandle, CURLOPT_LOW_SPEED_LIMIT, (long) mConfig->lowSpeedLimit);
        curl_easy_setopt(mHttpHandle, CURLOPT_LOW_SPEED_TIME, (long) mConfig->lowSpeedTimeMs / 1000);
      }

      if (mConfig->connectTimeoutMs) {
        SN_LOGD("set connect time to %d(ms)", mConfig->connectTimeoutMs);
        curl_easy_setopt(mHttpHandle, CURLOPT_CONNECTTIMEOUT, (long) mConfig->connectTimeoutMs / 1000);
      }
    }
    setEasyHandleCommonOpt();
    SN_LOGI("Create curlConnection:%p", this);
  }


  CurlConnection::~CurlConnection() {
    if (mHttpHandle) {
      curl_easy_cleanup(mHttpHandle);
      mHttpHandle = nullptr;
    }
    if (mResponseHeader) {
      delete[] mResponseHeader;
      mResponseHeader = nullptr;
    }
    SN_LOGI("~CurlConnection:%p", this);
  }

  void CurlConnection::setResume(int64_t pos) {
    if (pos < 0) {
      SN_LOGW("setResume: pos =%ld, is a illegal parameter, it will to be revised:0", pos);
      pos = 0;
    }
    mFilePos = pos;
    if (sendRange && this->mFilePos == 0) {
      curl_easy_setopt(mHttpHandle, CURLOPT_RANGE, "0-");
    } else {
      curl_easy_setopt(mHttpHandle, CURLOPT_RANGE, NULL);
      sendRange = 0;
    }
    curl_easy_setopt(mHttpHandle, CURLOPT_RESUME_FROM_LARGE, (curl_off_t) mFilePos);
  }

  void CurlConnection::setSource(const std::string &uri, curl_slist *headerList) {
    if (headerList) {
      curl_easy_setopt(mHttpHandle, CURLOPT_HTTPHEADER, headerList);
    } else {
      curl_easy_setopt(mHttpHandle, CURLOPT_HTTPHEADER, NULL);
    }
    mUri = uri;
    curl_easy_setopt(mHttpHandle, CURLOPT_URL, uri.c_str());

    //TODO:Sivin
//        CURLSH *sh = nullptr;
//        if (reSolveList) {
//            curl_slist_free_all(reSolveList);
//        }
//
//        reSolveList = CURLShareInstance::Instance()->getHosts(uri, &sh);
//        assert(sh != nullptr);
//        curl_easy_setopt(mHttpHandle, CURLOPT_SHARE, sh);
//
//        if (reSolveList != nullptr) {
//            curl_easy_setopt(mHttpHandle, CURLOPT_RESOLVE, reSolveList);
//        }
  }

  void CurlConnection::setPost(bool isPost, const uint8_t *data, int64_t size) {
    if (isPost) {
      curl_easy_setopt(mHttpHandle, CURLOPT_POST, 1);
      curl_easy_setopt(mHttpHandle, CURLOPT_POSTFIELDS, data);
      curl_easy_setopt(mHttpHandle, CURLOPT_POSTFIELDSIZE, (long) size);
    } else {
      curl_easy_setopt(mHttpHandle, CURLOPT_POST, 0);
    }
  }

  void CurlConnection::setInterrupt(const std::atomic_bool &interrupt) {
    mInterrupted.store(interrupt);
  }

  int CurlConnection::startConnect() {
    addToManager();
    int ret = 0;
    if ((ret = fillBuffer(1, mNeedReconnect)) < 0) {
      SN_LOGE("Connect, didn't get any data from stream.");
      return ret;
    }

    long responseCode;
    if (CURLE_OK == curl_easy_getinfo(mHttpHandle, CURLINFO_RESPONSE_CODE, &responseCode)) {
      SN_LOGD("CURLINFO_RESPONSE_CODE is %d", responseCode);
      if (responseCode >= 400) {
        return -1;
      }
    }
    calcFileSize();
    return 0;
  }

  int64_t CurlConnection::readBuffer(void *outBuffer, int64_t size) {
    std::lock_guard<std::mutex> lockGuard{mMutex};
    int64_t want = std::min(mBuffer->getReadableSize(), size);
    if (want > 0 && mBuffer->readData((char *) outBuffer, want) == want) {
      mFilePos += want;
      return want;
    }
    /* check if we finished prematurely */
    if (!mStillRunning && (mFileSize > 0 && mFilePos != mFileSize)) {
      SN_LOGE("%s - Transfer ended before entire file was retrieved pos %lld, size %lld", __FUNCTION__, mFilePos,
              mFileSize);
    }
    return 0;
  }

  int64_t CurlConnection::shortSeek(int64_t seekPos) {
    int64_t delta = seekPos - mFilePos;
    std::lock_guard<std::mutex> lockGuard{mMutex};
    //回退seek,无论失败还是成功则都直接返回
    if (delta < 0) {
      if (mBuffer->skipBytes(delta) > 0) {
        mFilePos = seekPos;
        return 0;
      }
      return -1;
    }

    //向前seek
    if (mBuffer->skipBytes(delta)) {
      mFilePos = seekPos;
      return 0;
    }

    //向前seek失败，表示可读取空间小于delta
    uint32_t shortBufferSize = 1024 * 64;//64k
    //判断off是否超过设定的阈值,如果超过阈值，则直接判定为无法shortSeek,return -1;
    //如果没有超过阈值，可以进行短暂的等待，即可到达指定的位置
    if (seekPos < mFilePos + shortBufferSize) {
      int64_t ringBufReadSize = mBuffer->getReadableSize();
      //先seek缓存的位置
      if (ringBufReadSize > 0) {
        mBuffer->skipBytes(ringBufReadSize);
        mFilePos += ringBufReadSize;
      }
      int ret = 0; //等待数据填充
      if ((ret = fillBuffer(shortBufferSize, mNeedReconnect)) < 0) {
        //数据填充失败,撤销之前的回退
        if (ringBufReadSize && !mBuffer->skipBytes(-ringBufReadSize)) {
          SN_LOGE("%s - Failed to restore position after failed filled", __FUNCTION__);
        } else {
          //撤销成功
          mFilePos -= ringBufReadSize;
        }
        SN_LOGW("forward short seek failed, because can not get enough data");
        return ret;
      }

      //填充成功
      SN_LOGI("read buffer size = %lld, need is %ld", mBuffer->getReadableSize(), (delta - ringBufReadSize));
      if (!mBuffer->skipBytes((delta - ringBufReadSize))) {
        SN_LOGE("%s - Failed to skip to position after having filled buffer", __FUNCTION__);
        if (ringBufReadSize && !mBuffer->skipBytes(-ringBufReadSize)) {
          SN_LOGE("%s - Failed to restore position after failed filled", __FUNCTION__);
        } else {
          //撤销成功
          mFilePos -= ringBufReadSize;
        }
        return -1;
      }
      mFilePos = seekPos;
      return 0;
    }
    SN_LOGW("forward seek failed, delta is too large");
    return -1;
  }

  //待优化重连次数
  int CurlConnection::fillBuffer(int64_t want, const std::atomic<bool> &needReconnect) {
    int64_t startTime = SNTimer::getSteadyTimeMs();
    //需要继续等待填充数据
    while (mBuffer->getReadableSize() < want && mBuffer->getWriteableSize() > 0) {

      //表示当前连接被中断，或者需要重连，则本次数据填充结束
      if (mInterrupted || needReconnect) {
        SN_LOGE("connect mInterrupted or needReconnect:%d,%ld", mInterrupted.load(), needReconnect.load());
        return -1;
      }
      if (mEos) { //流结束，直接返回填充数据成功
        SN_LOGI("stream end ...");
        return 0;
      }
      if (mPaused && mBuffer->getWriteableSize() > CURL_READ_BUFFER_SIZE) {
        mPaused = false;
        mConnectionManager->resumeConnection(shared_from_this());
      }

      //如果当前连接结束，或者发生错误，mStatus将会被赋值
      CURLcode code = mStatus;

      bool isError = code != CURLE_OK;
      switch (code) {
        case CURLE_OK:
          break;
        case CURLE_OPERATION_TIMEDOUT:
        case CURLE_PARTIAL_FILE:
        case CURLE_COULDNT_CONNECT:
        case CURLE_RECV_ERROR:
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_HTTP2: //已上错误码认为不是错误
          isError = false;
          break;
        case CURLE_HTTP_RETURNED_ERROR:
          long httpCode;
          curl_easy_getinfo(mHttpHandle, CURLINFO_RESPONSE_CODE, &httpCode);
          break;
        default:
          break;
      }

      if (isError) {
        //网络连接错误，无法读取到want足够多的数据
        //但缓冲区里还有数据，同样需要被读取，这里返回true
        if (mBuffer->getReadableSize() > 0) {
          return 0;
        }
        return -1;
      }

      //处理不等于OK，但但不是错误情况
      if (code != CURLE_OK) {
        //connectManger内部会会同步删除出错或者流结束的connection
        reset();
        if (mConfig && mConfig->listener) {
          //网络重连通知回调给外部
          IDataSource::Listener::NetWorkRetryStatus status;
          do {
            status = mConfig->listener->onNetWorkRetry(getErrorCode(code));
            if (mInterrupted) {
              return -1;
            }
            SNTimer::sleepMs(10);
          } while (status == IDataSource::Listener::NETWORK_RETRY_STATUS_PENDING);

        } else if (mConfig && SNTimer::getSteadyTimeMs() - startTime > mConfig->connectTimeoutMs) {
          mFilePos = 0;
          return -1;
        }

        SNTimer::sleepMs(10);
        setResume(mFilePos);
        applyReconnect(true);
        addToManager();
        continue;
      }

      if (isFirstLoop) {
        curl_off_t length = 0;
        if (curl_easy_getinfo(mHttpHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &length) == CURLE_OK) {
          if (length > 0) {
            if (mConfig && mConfig->listener) {
              mConfig->listener->onNetWorkConnected();
            }
            isFirstLoop = false;
          }
        }
      }
    }
    return 0;
  }

  void CurlConnection::reset() {
    if (mBuffer) {
      mBuffer->clear();
    }

    mEos = false;
    mStatus = CURLE_OK;

    mResponseHeaderSize = 0;
    delete[] mResponseHeader;
    mResponseHeader = nullptr;
  }

  void CurlConnection::clearCurlOpt() {
    if (mHttpHandle) {
      curl_easy_setopt(mHttpHandle, CURLOPT_VERBOSE, FALSE);
      curl_easy_setopt(mHttpHandle, CURLOPT_WRITEDATA, nullptr);
      curl_easy_setopt(mHttpHandle, CURLOPT_HEADERDATA, nullptr);
      curl_easy_setopt(mHttpHandle, CURLOPT_SOCKOPTDATA, nullptr);
      curl_easy_setopt(mHttpHandle, CURLOPT_DEBUGDATA, nullptr);
      curl_easy_setopt(mHttpHandle, CURLOPT_XFERINFODATA, nullptr);
    }
  }

  int CurlConnection::closeConnection(bool forbidReuse) {
    if (forbidReuse) {
      curl_easy_setopt(mHttpHandle, CURLOPT_FORBID_REUSE, 1);
    }
    removeFromManager();
    return 0;
  }

  void CurlConnection::setEasyHandleCommonOpt() {
    curl_easy_setopt(mHttpHandle, CURLOPT_VERBOSE, TRUE);
    curl_easy_setopt(mHttpHandle, CURLOPT_FOLLOWLOCATION, 1L);
    //最大重定向次数
    curl_easy_setopt(mHttpHandle, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(mHttpHandle, CURLOPT_NOSIGNAL, 1L);
    //设置curl接收缓冲区大小，默认是16k，最大不能超过512k
    curl_easy_setopt(mHttpHandle, CURLOPT_BUFFERSIZE, CURL_READ_BUFFER_SIZE);
    //如果设置为1则会阻止xferinfo函数调用，
    curl_easy_setopt(mHttpHandle, CURLOPT_NOPROGRESS, 0);

    curl_easy_setopt(mHttpHandle, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(mHttpHandle, CURLOPT_WRITEDATA, this);

    curl_easy_setopt(mHttpHandle, CURLOPT_HEADERFUNCTION, headerCallback);
    curl_easy_setopt(mHttpHandle, CURLOPT_HEADERDATA, this);

    curl_easy_setopt(mHttpHandle, CURLOPT_SOCKOPTFUNCTION, sockoptCallback);
    curl_easy_setopt(mHttpHandle, CURLOPT_SOCKOPTDATA, this);

    curl_easy_setopt(mHttpHandle, CURLOPT_XFERINFOFUNCTION, xferinfo);
    curl_easy_setopt(mHttpHandle, CURLOPT_XFERINFODATA, this);

    curl_easy_setopt(mHttpHandle, CURLOPT_DEBUGFUNCTION, connectTrace);
    curl_easy_setopt(mHttpHandle, CURLOPT_DEBUGDATA, this);

    curl_easy_setopt(mHttpHandle, CURLOPT_SSL_VERIFYPEER, FALSE);
    // openssl not verify host, otherwise will return CURLE_PEER_FAILED_VERIFICATION
    curl_easy_setopt(mHttpHandle, CURLOPT_SSL_VERIFYHOST, FALSE);
  }

  size_t CurlConnection::writeCallback(char *buffer, size_t size, size_t nmemb, void *userdata) {
    if (!userdata) {
      return CURL_WRITEFUNC_PAUSE;
    }
    auto *connect = (CurlConnection *) userdata;
    auto amount = (uint32_t) (size * nmemb);

    assert(!connect->mPaused);

    std::lock_guard<std::mutex> lock{connect->mMutex};
    //缓存空间小于传输数据，则暂停传输
    if (connect->mBuffer->getWriteableSize() < amount) {
      connect->mPaused = true;
      return CURL_WRITEFUNC_PAUSE;
    }

    if (connect->mBuffer->writeData(buffer, amount) != amount) {
      SN_LOGE("write ring buffer error %u %u", amount, connect->mBuffer->getWriteableSize());
      assert(0);
    }

    if (connect->mConfig && connect->mConfig->listener) {
      connect->mConfig->listener->onNetWorkInput(amount, IDataSource::Listener::BITSTREAM_TYPE_MEDIA);
    }
    return amount;
  }

  size_t CurlConnection::headerCallback(char *buffer, size_t size, size_t nitems, void *userdata) {
    if (!userdata) {
      return size * nitems;
    }
    auto *connection = (CurlConnection *) userdata;
    if (connection->mResponseHeader == nullptr) {
      connection->mResponseHeader = new char[MAX_HEADER_SIZE];
      memset(connection->mResponseHeader, 0, MAX_HEADER_SIZE);
      connection->mResponseHeaderSize = 0;
      SN_LOGI("create mResponseHeader...");
    }

    if (connection->mResponseHeaderSize + size * nitems < MAX_HEADER_SIZE) {
      memcpy(connection->mResponseHeader + connection->mResponseHeaderSize, buffer, size * nitems);
      connection->mResponseHeaderSize += size * nitems;
    } else {
      SN_LOGE("responseHeader filed can not hold header response: "
              "headerSize = %ld, size * nitems = %ld, maxHaderSize = %ld",
              connection->mResponseHeaderSize, size * nitems, MAX_HEADER_SIZE);
    }
    return size * nitems;
  }

  int CurlConnection::sockoptCallback(void *clientp, curl_socket_t curlfd, curlsocktype purpose) {
    auto *connect = (CurlConnection *) clientp;
    if (purpose == CURLSOCKTYPE_IPCXN) { //表示socket创建成功，CURLSOCKTYPE_IPCXN表示主动创建
      int rcv_buffer = connect->mConfig ? connect->mConfig->sockReceiveBufferSize : 0;
      if (rcv_buffer > 0) {
        setsockopt(curlfd, SOL_SOCKET, SO_RCVBUF, (const char *) &rcv_buffer, sizeof(rcv_buffer));
      }

      if (connect->mListener) {
        connect->mListener->onDNSResolved();
      }
      connect->mDNSResolved = true;
    }
    return CURL_SOCKOPT_OK;
  }

  int CurlConnection::connectTrace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp) {
    const char *text;
    auto connect = (CurlConnection *) userp;
    switch (type) {
      case CURLINFO_TEXT:
//                SN_LOGD("curl info : %s", data);
        break;

      case CURLINFO_HEADER_OUT:
        text = "=> Send header";
        break;
      case CURLINFO_DATA_OUT:
        text = "=> Send data";
        break;
      case CURLINFO_SSL_DATA_OUT:
        text = "=> Send SSL data";
        break;
      case CURLINFO_HEADER_IN:
        text = "<= Recv header";
        break;
      case CURLINFO_DATA_IN:
        text = "<= Recv data";
        break;
      case CURLINFO_SSL_DATA_IN:
        text = "<= Recv SSL data";
        break;
      default:
        return 0;
    }
    return 0;
  }

  //CURL 传输数据回调：
  //dltotal:期望这次传输中下载的总字节数，dlnow目前已经下载的字节数，ultotal:期望上传的总字节数，ulnow：目前已经上传的总字节数
  int CurlConnection::xferinfo(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal,
                               curl_off_t ulnow) {
    if (!clientp) {
      return 0;
    }
    auto *connect = (CurlConnection *) clientp;
    //暂停结束
    if (connect->mPaused && connect->mBuffer->getWriteableSize() > CURL_READ_BUFFER_SIZE) {
      connect->mPaused = false;
      curl_easy_pause(connect->mHttpHandle, CURLPAUSE_CONT);
    }
    return 0;
  }

  void CurlConnection::applyReconnect(bool reconnect) {
    curl_easy_setopt(mHttpHandle, CURLOPT_FRESH_CONNECT, reconnect);
  }

  void CurlConnection::addToManager() {
    mConnectionManager->addConnection(shared_from_this());
    mStillRunning = true;
  }

  void CurlConnection::removeFromManager() {
    mConnectionManager->removeConnection(shared_from_this());
  }

  void CurlConnection::calcFileSize() {
    if (mFileSize < 0) {
      curl_off_t length;
      if (CURLE_OK == curl_easy_getinfo(mHttpHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &length)) {
        if (length < 0) {
          length = 0;
        }
        if (length > 0) {
          mFileSize = mFilePos + (int64_t) length;
        } else {
          mFileSize = -1;
        }
      }
    }
  }

  void CurlConnection::onConnectOver(bool eos, CURLcode code) {
    mEos = eos;
    mStatus = code;
    mStillRunning = false;
  }

  CURL *CurlConnection::getCurlHandle() const {
    return mHttpHandle;
  }

  int CurlConnection::getFileSize() {
    if (!mStillRunning) return -1;
    return mFileSize;
  }

  bool CurlConnection::needReconnect() {
    return mNeedReconnect;
  }

  void CurlConnection::setReconnect(bool needReconnect) {
    mNeedReconnect = needReconnect;
  }

  int CurlConnection::readCheck(bool needReconnect) {
    return fillBuffer(1, needReconnect);
  }


}