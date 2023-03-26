//
// Created by Sivin on 2022-11-26.
//
#include "MediaPlayerUtil.h"
#include <memory>
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

  int64_t SnPlayer::getPlayerBufferDuration(bool gotMax, bool internal) {

    return 0;
  }

  void SnPlayer::doReadPacket() {
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
      //TODO: 1000 * 1000的数字含义不明确
      if ((0 >= checkStep--) && (curBufferDuration > 1000 * 1000)) {
        
      }
    }
  }


}// namespace Sivin