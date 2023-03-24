//
// Created by sivin on 23-1-14.
//
#define LOG_TAG "PlayerMsgProcessor"

#include "PlayerMsgProcessor.h"
#include "utils/SNLog.h"

namespace Sivin {

  PlayerMsgProcessor::PlayerMsgProcessor(SnPlayer &player) : mPlayer(player) {}


  void PlayerMsgProcessor::processSetDataSourceMsg(const std::string &url) {
    if (mPlayer.mPlayStatus == PlayerStatus::IDLE || mPlayer.mPlayStatus == PlayerStatus::STOPPED) {
      mPlayer.mParams->url = url;
      mPlayer.changePlayerStatus(PlayerStatus::INITIALIZED);
    }
  }

  void PlayerMsgProcessor::processPrepareMsg() {

  }


  void PlayerMsgProcessor::openUrl() {

  }


}