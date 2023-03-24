//
// Created by Sivin on 2022-11-26.
//
#define LOG_TAG "SnPlayer"

#include "SnPlayer.h"
#include "MediaPlayerDef.h"
#include "PlayerMsgController.h"
#include "PlayerMsgProcessor.h"
#include "PlayerParams.h"
#include "utils/SNLog.h"
#include "utils/SNTimer.h"

static Sivin::PlayerMsg emptyMsg{};

namespace Sivin {
  SnPlayer::SnPlayer() {
    SN_TRACE;
    mParams = std::make_unique<PlayerParams>();
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

  void SnPlayer::prepare() {
    if (mPlayStatus != PlayerStatus::INITIALIZED && mPlayStatus != PlayerStatus::STOPPED) {
      stop();
    }
    std::unique_lock<std::mutex> lock{mPlayerMutex};
    putMsg(PlayerMsgType::PREPARE, emptyMsg, true);
    mPlayerThread->start();
  }

  void SnPlayer::start() {}

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
    if (mPlayStatus != newStatus) {}
  }


  int SnPlayer::mainService() {
    if (mCanceled.load()) {
      return 0;
    }

    int64_t curTime = SNTimer::getSteadyTimeMs();
    //    mUtil->notifyPlayerLoop(curTime);
    //    sendDCAMessage();
    if (mMsgController->empty() || mMsgController->processMsg() == 0) {
      // TODO:干什么？
      processVideoLoop();
    }

    return 0;
  }

  void SnPlayer::processVideoLoop() {
    int64_t curTime = SNTimer::getSteadyTimeMs() / 1000;
    if (mPlayStatus != PlayerStatus::COMPLETION &&
            (mPlayStatus < PlayerStatus::PLAYING || mPlayStatus > PlayerStatus::PAUSED) ||
        mDemuxerService == nullptr) {
    }
  }

}// namespace Sivin