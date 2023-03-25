//
// Created by sivin on 23-1-14.
//

#ifndef SIVINPLAYER_PLAYERMSGPROCESSOR_H
#define SIVINPLAYER_PLAYERMSGPROCESSOR_H

#include "PlayerMsgController.h"
#include "SnPlayer.h"

namespace Sivin {

  /**
    处理播放器转发的消息类
  */
  class PlayerMsgProcessor : public IPlayerMsgProcessor {

  public:
    explicit PlayerMsgProcessor(SnPlayer &player);

  private:
    void processSetDataSourceMsg(const std::string &url) override;

    void processPrepareMsg() override;


  private:
    int openUrl();

  private:
    SnPlayer &mPlayer;
  };
}


#endif //SIVINPLAYER_PLAYERMSGPROCESSOR_H
