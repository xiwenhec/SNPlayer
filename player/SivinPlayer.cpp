//
// Created by Sivin on 2022-11-26.
//
#define LOG_TAG "SivinPlayer"

#include "SivinPlayer.h"
#include "PlayerMsgProcessor.h"
#include "utils/SNTimer.h"


namespace Sivin {

  SivinPlayer::SivinPlayer() {
    mParamsSet = std::make_unique<PlayerParamsSet>();
    mMsgProcessor = std::make_unique<PlayerMsgProcessor>(*this);
    mMsgController = std::make_unique<PlayerMsgController>(*mMsgProcessor);
    mThread = MAKE_UNIQUE_THREAD(mainService, LOG_TAG);
  }

  SivinPlayer::~SivinPlayer() {
  }

  void SivinPlayer::putMsg(PlayerMsgType msgType, const PlayerMsgContent &msgContent, bool trigger) {
    mMsgController->putMsg(msgType, msgContent);
  }

  void SivinPlayer::setDataSource(const char *url) {

  }


  void SivinPlayer::changePlayerStatus(PlayerStatus newStatus) {
    mOlderStatus = mStatus;
    if (mStatus != newStatus) {

    }
  }

  int SivinPlayer::mainService() {
    mMainServiceCanceled = mCanceled.load();
    if (mCanceled) return 0;
    int64_t curTime = SNTimer::getSteadyTimeMs();
//    mUtil->notifyPlayerLoop(curTime);
//    sendDCAMessage();
    if (mMsgController->empty() || mMsgController->processMsg() == 0) {
      //TODO:干什么？
      processVideoLoop();

    }

    return 0;
  }

  void SivinPlayer::processVideoLoop() {
    int64_t curTime = SNTimer::getSteadyTimeMs() / 1000;
    if (mStatus != PlayerStatus::COMPLETION &&
        (mStatus < PlayerStatus::PLAYING || mStatus > PlayerStatus::PAUSED) || mDemuxerService == nullptr) {

    }


  }


} // Sivin