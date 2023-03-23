//
// Created by sivin on 23-1-14.
//
#define LOG_TAG "PlayerMsgProcessor"

#include "PlayerMsgProcessor.h"
#include "utils/SNLog.h"

namespace Sivin {

  PlayerMsgProcessor::PlayerMsgProcessor(SivinPlayer &player) : mPlayer(player) {}


  void PlayerMsgProcessor::processSetDataSourceMsg(const std::string &url) {
    if (mPlayer.mStatus == PlayerStatus::IDLE || mPlayer.mStatus == PlayerStatus::STOPPED) {
      mPlayer.mParamsSet->url = url;
      mPlayer.changePlayerStatus(PlayerStatus::INITIALIZED);
    }
  }

  void PlayerMsgProcessor::processPrepareMsg() {

  }


  void PlayerMsgProcessor::openUrl() {

  }


}