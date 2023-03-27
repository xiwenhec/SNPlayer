//
// Created by Sivin on 2022-11-26.
//
#include "MediaPlayerUtil.h"
#include "base/error/SNError.h"
#include "base/media/SNPacket.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>
#define LOG_TAG "SnPlayer"

#include <cstdint>
#include "SnPlayer.h"
#include "MediaPlayerDef.h"
#include "PlayerMsgController.h"
#include "PlayerMsgProcessor.h"
#include "PlayerParams.h"
#include "utils/SNLog.h"
#include "utils/SNTimer.h"

#define HAVE_VIDEO (mCurrentVideoIndex >= 0)
#define HAVE_AUDIO (mCurrentAudioIndex >= 0)

static Sivin::PlayerMsg emptyMsg{};

namespace Sivin {
  SnPlayer::SnPlayer() {
    SN_TRACE;
    mParams = std::make_unique<PlayerParams>();
    mUtil = std::make_unique<MediaPlayerUtil>();
    mBufferController = std::make_unique<BufferController>();
    mMsgProcessor = std::make_unique<PlayerMsgProcessor>(*this);
    mMsgController = std::make_unique<PlayerMsgController>(*mMsgProcessor);
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
  }

  void SnPlayer::resetSeekStatus() {
    mSeekPos = -1;
  }


  /**
    获取封装缓冲队列中的数据缓冲时长
    gotMax:true,获取音频，视频，字幕队列中的最大时长,false:返回最小时长

  */
  int64_t SnPlayer::getPlayerBufferDuration(bool gotMax, bool internal) {
    int64_t videoBufDuration = -1;
    int64_t audioBufDuration = -1;

    if (HAVE_AUDIO) {

      //获取播放器缓冲队列里的时长
      audioBufDuration = mBufferController->getPacketDuration(BufferType::BUFFER_TYPE_AUDIO);

      

    }


    return 0;
  }

  void SnPlayer::readPacket() {
    if (mEof) {
      return;
    }

    int64_t curBufferDuration = getPlayerBufferDuration(false, false);
    mUtil->notifyRead(MediaPlayerUtil::PacketReadEvent::EVENT_LOOP, 0);

    int64_t readStartTime = SNTimer::getSteadyTimeUs();
    int checkStep = 0;
    while (true) {
      if (mBufferIsFull) {
        //TODO:缓冲区慢时的处理
      }

      if (curBufferDuration > mParams->maxBufferDuration &&
          getPlayerBufferDuration(false, true) > mParams->startBufferDuration) {
        mBufferIsFull = true;
        break;
      }
      mBufferIsFull = false;

      int ret = doReadPacket();
      if (ret == 0) {//
        if (mPlayStatus == PlayerStatus::PREPARING) {
          if (HAVE_VIDEO && !mHaveVideoPkt) {
            closeVideo();
          }
          if (HAVE_AUDIO && !mHaveAudioPkt) {
            closeAudio();
          }
        }
        break;

      } else if (ret == SNRET_ERROR) {
        break;

      } else if (ret == SNRET_AGAIN) {
        //TODO:处理读取数据包again时的情况
        break;
      }
    }
  }

  //主要任务：从解封装服务中读取压缩数据加入缓存队列
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

    if (packet->getInfo().streamIndex == mCurrentVideoIndex) {
      mHaveVideoPkt = true;
      mBufferController->addPacket(std::move(packet), BufferType::BUFFER_TYPE_VIDEO);
    } else if (packet->getInfo().streamIndex == mCurrentAudioIndex) {
      mHaveAudioPkt = true;
      mBufferController->addPacket(std::move(packet), BufferType::BUFFER_TYPE_AUDIO);
    }
    return ret;
  }

}// namespace Sivin