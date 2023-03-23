//
// Created by sivin on 23-1-14.
//

#ifndef SIVINPLAYER_PLAYERMSGPROCESSOR_H
#define SIVINPLAYER_PLAYERMSGPROCESSOR_H

#include "PlayerMsgController.h"
#include "SivinPlayer.h"

namespace Sivin {

  class PlayerMsgProcessor : public IPlayerMsgProcessor {

  public:
    explicit PlayerMsgProcessor(SivinPlayer &player);

  private:
    void processSetDataSourceMsg(const std::string &url) override;

    void processPrepareMsg() final;


  private:
    void openUrl();


  private:
    SivinPlayer &mPlayer;
  };
}


#endif //SIVINPLAYER_PLAYERMSGPROCESSOR_H
