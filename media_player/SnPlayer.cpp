//
// Created by Sivin on 2022-11-26.
//

#define LOG_TAG "SnPlayer"

#include "SnPlayer.h"
#include "DeviceManager.h"
#include "MediaPacketQueue.h"
#include "PlayerNotifier.h"
#include "base/error/SNError.h"
#include "base/media/SNPacket.h"
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <utility>
#include <cstdint>
#include "MediaPlayerDef.h"
#include "PlayerMsgController.h"
#include "PlayerMsgProcessor.h"
#include "PlayerParams.h"
#include "utils/SNLog.h"
#include "utils/SNTimer.h"
#include "utils/os/SNSysInfoUtil.h"

#define HAVE_VIDEO (mCurrentVideoIndex >= 0)
#define HAVE_AUDIO (mCurrentAudioIndex >= 0)

#define VIDEO_PICTURE_MAX_CACHE_SIZE 2

namespace Sivin {

  //表示一秒钟
  static const int kSecond = 1000 * 1000;
  static const int kBufferGap = 1 * kSecond;

  //表示1M字节
  static const int k1M = 1024 * 1024;

  //一帧视频渲染的默认时长(us)
  static const int VIDEO_RENDER_DURATION_DEFAULT = 40 * 1000;
  static const int AUDIO_RENDER_DURATION_DEFAULT = 23 * 1000;


  static PlayerMsg emptyMsg{};

  SnPlayer::SnPlayer() {
    SN_TRACE;
    mParams = std::make_unique<PlayerParams>();
    mStat = std::make_unique<PlayerStatistic>();
    mBufferController = std::make_unique<BufferController>();
    mMsgProcessor = std::make_unique<PlayerMsgProcessor>(*this);
    mMsgController = std::make_unique<PlayerMsgController>(*mMsgProcessor);
    mDeviceManager = std::make_unique<DeviceManager>();
    mNotifier = std::make_unique<PlayerNotifier>();
    mPlayerThread = MAKE_UNIQUE_THREAD(mainService, LOG_TAG);
  }

  SnPlayer::~SnPlayer() {}


  void SnPlayer::setDataSource(const char *url) {
    SN_TRACE;
    PlayerMsg msg{};
    PlayerDataSourceMsg datasourceMsg;
    datasourceMsg.url = new std::string{url};
    msg.dataSource = datasourceMsg;
    putMsg(PlayerMsgType::SET_DATASOURCE, msg, 0);
  }

  void SnPlayer::setView(void *view) {}

  void SnPlayer::setListener(const PlayerListener &listener) {
    mListener = listener;
  }

  void SnPlayer::prepare() {
    if (mPlayStatus != PlayerStatus::INITIALIZED && mPlayStatus != PlayerStatus::STOPPED) {
      stop();
    }
    std::unique_lock<std::mutex> lock{mPlayerMutex};
    putMsg(PlayerMsgType::PREPARE, emptyMsg, true);
    mPlayerThread->start();
  }

  void SnPlayer::start() {
    SN_TRACE;
  }

  void SnPlayer::pause() {
  }
  void SnPlayer::seekTo(int64_t seekPos, bool isAccurate) {
  }
  void SnPlayer::stop() {
  }

  void SnPlayer::putMsg(PlayerMsgType msgType, const PlayerMsg &msg, bool trigger = 0) {
    mMsgController->putMsg(msgType, msg);
  }


  void SnPlayer::changePlayerStatus(PlayerStatus newStatus) {
    mOlderStatus = mPlayStatus;
    SN_LOGD("change status: newStatus = %d, oldStatus = %ld\n", newStatus, mOlderStatus);
    if (mPlayStatus != newStatus) {
      mPlayStatus = newStatus;
    }
  }


  int SnPlayer::mainService() {
    if (mCanceled.load()) {
      return 0;
    }

    int64_t curTime = SNTimer::getSteadyTimeMs();
    //首先处理需要待处理的消息
    if (mMsgController->empty() || mMsgController->processMsg() == 0) {
      processVideoLoop();
    }


    return 0;
  }

  void SnPlayer::processVideoLoop() {

    int64_t curTime = SNTimer::getSteadyTimeMs() / 1000;

    if (mPlayStatus != PlayerStatus::COMPLETION &&
            (mPlayStatus < PlayerStatus::PREPARING || mPlayStatus > PlayerStatus::PAUSED) ||
        mDemuxerService == nullptr) {
      return;//当前播放器处于不可播放状态
    }

    readPacket();
    decodePacket();
  }

  void SnPlayer::resetSeekStatus() {
    mSeekPos = -1;
  }


  /**
    获取封装缓冲队列中的数据缓冲时长
    gotMax:true,获取音频，视频，字幕队列中的最大时长,false:返回最小时长

  */
  int64_t SnPlayer::getPlayerBufferDuration(bool gotMax) {

    int64_t videoBufDuration = -1;
    int64_t audioBufDuration = -1;

    if (HAVE_AUDIO) {
      //获取播放器缓冲队列里的时长
      audioBufDuration = mBufferController->getPacketDuration(BufferType::AUDIO);
      if (mDeviceManager->isDecoderValid(DeviceType::AUDIO)) {
        //TODO:这里计算不精确
        audioBufDuration += mDeviceManager->getDecoder(DeviceType::AUDIO)->getInputQueueSize() * AUDIO_RENDER_DURATION_DEFAULT;
      }
      if (mParams->preferAudio) {
        return audioBufDuration;
      }
    }

    if (HAVE_VIDEO) {
      videoBufDuration = mBufferController->getPacketDuration(BufferType::VIDEO);

      if (videoBufDuration < 0 && !HAVE_AUDIO) {
        videoBufDuration = mBufferController->getLastPacketPts(BufferType::VIDEO) -
                           mBufferController->getFirstPacketPts(BufferType::VIDEO);
      }

      if (videoBufDuration < 0) {
        videoBufDuration = mBufferController->getPacketCount(BufferType::VIDEO) * VIDEO_RENDER_DURATION_DEFAULT;
      }

      //TODO:这里的获取时不准确的
      if (mDeviceManager->isDecoderValid(DeviceType::VIDEO)) {
        videoBufDuration += mDeviceManager->getDecoder(DeviceType::VIDEO)->getInputQueueSize() * VIDEO_RENDER_DURATION_DEFAULT;
      }
    }

    //比较音频和视频队列的缓冲时长
    int64_t maxVal, minVal;
    if (audioBufDuration > videoBufDuration) {
      maxVal = audioBufDuration;
      minVal = videoBufDuration;
    } else {
      maxVal = videoBufDuration;
      minVal = audioBufDuration;
    }
    return gotMax ? maxVal : minVal;
  }


  void SnPlayer::readPacket() {
    if (mReadEos) {
      return;
    }
    mStat->stat(StatisticEvent::LOOP, 0);

    SNSysInfo sysInfo;
    int checkStep = 0;

    int64_t readStartTime = SNTimer::getSteadyTimeUs();

    while (true) {

      int64_t curBufferDuration = getPlayerBufferDuration(false);

      if (mBufferIsFull) {
        //上一次buffer已经读满，如果距离本次读取这段时间消费的数据超过一个bufferGap
        //则表示需要再次读取，否则不需要读取，直接返回
        if (mParams->maxBufferDuration > 2 * kBufferGap &&
            (curBufferDuration > mParams->maxBufferDuration - kBufferGap) &&
            curBufferDuration > mParams->startBufferDuration) {
          break;
        }
      }

      if (curBufferDuration > mParams->maxBufferDuration &&
          curBufferDuration > mParams->startBufferDuration) {
        mBufferIsFull = true;
        break;
      }
      mBufferIsFull = false;

      //真正读取数据包前，对系统可用内存做检测
      if ((0 >= checkStep--) && (curBufferDuration > 1 * kSecond) && (SNSysInfoUtil::getSystemMemoryInfo(&sysInfo) >= 0)) {
        if (sysInfo.availableram > 2 * mParams->lowMemSize) {
          checkStep = (int) (sysInfo.availableram / (5 * k1M));
        } else if (sysInfo.availableram < mParams->lowMemSize) {
          //内存状态有良好->低内存
          if (!mLowMem) {
            //TODO:通知出现低内存报警
            mNotifier->notifyEvent(PlayerEventCode::LOW_MEMORY, "low memeory");
          }
          mLowMem = true;
          if (mParams->startBufferDuration > 800 * 1000) {
            mParams->startBufferDuration = 800 * 1000;
          }
          break;
        } else {
          checkStep = 5;
          mLowMem = false;
        }
      }

      //读取数据
      int ret = doReadPacket();

      if (ret == 0) {
        if (mPlayStatus == PlayerStatus::PREPARING) {
          if (HAVE_VIDEO && !mHaveVideoPkt) {
            closeVideo();
          }
          if (HAVE_AUDIO && !mHaveAudioPkt) {
            closeAudio();
          }
        }
        mReadEos = true;
        break;

      } else if (ret == SNRET_AGAIN) {
        //TODO:处理读取数据包again时的情况
        //mUtil->notifyRead(PacketReadEvent::AGAIN, 0);
        break;
      } else if (ret == SNRET_ERROR) {
        notifyError(PlayerError::RREAD_PACKET);
        break;
      }

      int timeout = 10000;
      if (SNTimer::getSteadyTimeUs() - readStartTime > timeout) {
        //TODO:处理读取超时
        break;
      }
    }//while
  }


  //从解封装服务中读取压缩数据加入缓存队列
  //失败：返回错误码<0，成功：返回读取的字节数, 0：读取到文件结尾
  int SnPlayer::doReadPacket() {
    assert(mDemuxerService != nullptr);
    std::unique_ptr<SNPacket> packet{};
    int ret = mDemuxerService->readPacket(packet, -1);
    if (packet == nullptr) {
      if (ret != SNRET_ERROR) {
        return SNRET_AGAIN;
      }
      return ret;
    }

    mStat->stat(StatisticEvent::GOT_PACKET, packet->getSize());

    if (packet->getInfo().streamIndex == mCurrentVideoIndex) {
      mHaveVideoPkt = true;
      mBufferController->addPacket(std::move(packet), BufferType::VIDEO);

    } else if (packet->getInfo().streamIndex == mCurrentAudioIndex) {
      mHaveAudioPkt = true;
      mBufferController->addPacket(std::move(packet), BufferType::AUDIO);
    }
    return ret;
  }


  void SnPlayer::decodePacket() {

    if (HAVE_VIDEO && !mVideoDecoderEOS && mDeviceManager->isDecoderValid(DeviceType::VIDEO)) {

      int maxCacheSize = VIDEO_PICTURE_MAX_CACHE_SIZE;
      
      uint64_t videoFrameCount = mVideoFrameQue.size();
      
      if (videoFrameCount < maxCacheSize) {
        int64_t startDecodeTime = SNTimer::getSteadyTimeUs();
        do {
          if (mCanceled) {
            break;
          }
          if ((mAppStatus == AppStatus::BACKGROUND) && isSeeking()) {
            break;
          }
          if (mVideoPacket == nullptr) {
            mVideoPacket = mBufferController->getPacket(BufferType::VIDEO);
          }
          

        } while (true);
      }
    }
  }

  void SnPlayer::notifyError(PlayerError error) {
  }

}// namespace Sivin