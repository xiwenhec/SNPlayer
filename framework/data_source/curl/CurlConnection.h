//
// Created by sivin on 11/28/22.
//

#ifndef SN_FRAMEWORK_CURLCONNECTION_H
#define SN_FRAMEWORK_CURLCONNECTION_H

#include <memory>
#include <curl/curl.h>
#include <mutex>
#include "base/RingBuffer.h"
#include "data_source/IDataSource.h"

namespace Sivin {

  class CurlConnectionManager;

  class CurlConnection : public std::enable_shared_from_this<CurlConnection> {

    friend CurlConnectionManager;

    class ConnectionListener {
    public:
      virtual ~ConnectionListener() = default;

      virtual void onDNSResolved() {}
    };

  public:

    explicit CurlConnection(IDataSource::SourceConfig *config,
                            std::shared_ptr<CurlConnectionManager> connectionManager,
                            std::shared_ptr<ConnectionListener> listener);

    ~CurlConnection();

    void setSource(const std::string &uri, curl_slist *headerList);

    void setPost(bool isPost, const uint8_t *data, int64_t size);

    void setResume(int64_t pos);

    void setReconnect(bool needReconnect);

    bool needReconnect();

    int startConnect();

    void setInterrupt(const std::atomic_bool &interrupt);

    int64_t tell() const { return mFilePos; }

    int readCheck(bool needReconnect);

    int64_t readBuffer(void *outBuffer, int64_t size);

    int64_t shortSeek(int64_t off);

    int closeConnection(bool forbidReuse);

    int getFileSize();


  private:

    void setEasyHandleCommonOpt();

    void applyReconnect(bool reconnect);

    void addToManager();

    void removeFromManager();

    CURL *getCurlHandle() const;

    int fillBuffer(int64_t want, const std::atomic<bool> &needReconnect);

    /*
    * 连接出错或者结束
    * 当这个函数调用后，该connection->handle随后将会被移出multiHandle
    */
    void onConnectOver(bool eos, CURLcode code);

    void reset();

    void clearCurlOpt();

    void calcFileSize();

    static size_t writeCallback(char *buffer, size_t size, size_t nmemb, void *userdata);

    static size_t headerCallback(char *buffer, size_t size, size_t nitems, void *userdata);

    static int sockoptCallback(void *clientp, curl_socket_t curlfd, curlsocktype purpose);

    static int connectTrace(CURL *handle, curl_infotype type, char *data, size_t size, void *userp);

    static int xferinfo(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal,
                        curl_off_t ulnow);

  private:
    std::string mUri;
    IDataSource::SourceConfig *mConfig{nullptr};
    std::shared_ptr<ConnectionListener> mListener{nullptr};
    std::shared_ptr<CurlConnectionManager> mConnectionManager{nullptr};
    //当前connect内部数据缓冲
    std::shared_ptr<RingBuffer> mBuffer{nullptr};

    CURL *mHttpHandle{nullptr};

    bool mDNSResolved{false};

    //内部标识使用，当connection接受缓冲区满，则暂停curl
    //当connection空间可写，则继续传输.
    bool mPaused{false};

    //标识当前流是否结束
    bool mEos{false};

    std::atomic_bool mInterrupted{false};

    std::mutex mMutex;

    //响应头
    char *mResponseHeader{nullptr};
    int64_t mResponseHeaderSize = 0;

    CURLcode mStatus{CURLE_OK};

    bool isFirstLoop{false};
    int64_t mFilePos{0};
    int64_t mFileSize{-1};

    int sendRange{0};
    //当前connect需要重新连接
    bool mNeedReconnect{false};
    bool mStillRunning{false};

  };
}


#endif //SN_FRAMEWORK_CURLCONNECTION_H
