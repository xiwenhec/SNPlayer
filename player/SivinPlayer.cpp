//
// Created by Sivin on 2022-11-26.
//
#include "PlayerMsgController.h"
#define LOG_TAG "SivinPlayer"

#include "SivinPlayer.h"
#include "PlayerMsgProcessor.h"
#include "utils/SNTimer.h"


namespace Sivin {
  SivinPlayer::SivinPlayer() {
    mParams = std::make_unique<PlayerParams>();
    mMsgProcessor = std::make_unique<PlayerMsgProcessor>(*this);
    mMsgController = std::make_unique<PlayerMsgController>(*mMsgProcessor);
    mThread = MAKE_UNIQUE_THREAD(mainService, LOG_TAG);
  }

  SivinPlayer::~SivinPlayer() {}


  void SivinPlayer::setDataSource(const char *url) {
  }

  void SivinPlayer::setView(void *view) {}

  void SivinPlayer::prepare() {}

  void SivinPlayer::start() {}


  void SivinPlayer::putMsg(PlayerMsgType msgType, const PlayerMsg &msgContent, bool trigger = 0) {
    mMsgController->putMsg(msgType, msgContent);
  }


  void SivinPlayer::changePlayerStatus(PlayerStatus newStatus) {
    mOlderStatus = mStatus;
    if (mStatus != newStatus) {
    }
  }


  int SivinPlayer::mainService() {
    mMainServiceCanceled = mCanceled.load();
    if (mCanceled)
      return 0;
    int64_t curTime = SNTimer::getSteadyTimeMs();
    //    mUtil->notifyPlayerLoop(curTime);
    //    sendDCAMessage();
    if (mMsgController->empty() || mMsgController->processMsg() == 0) {
      // TODO:干什么？
      processVideoLoop();
    }

    return 0;
  }

  void SivinPlayer::processVideoLoop() {
    int64_t curTime = SNTimer::getSteadyTimeMs() / 1000;
    if (mStatus != PlayerStatus::COMPLETION &&
            (mStatus < PlayerStatus::PLAYING || mStatus > PlayerStatus::PAUSED) ||
        mDemuxerService == nullptr) {
    }
  }

}// namespace Sivin