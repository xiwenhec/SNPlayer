//
// Created by Sivin on 2022-11-26.
//

#ifndef SIVINPLAYER_SIVINPLAYER_H
#define SIVINPLAYER_SIVINPLAYER_H

#include "data_source/IDataSource.h"
#include "utils/SNThread.h"
#include "demuxer/DemuxerService.h"
#include "PlayerParamsSet.h"
#include "PlayerDefinition.h"
#include "PlayerMsgController.h"
#include "memory"


namespace Sivin {

  class SivinPlayer {
    friend class PlayerMsgProcessor;

  public:
    SivinPlayer();

    ~SivinPlayer();

  public:
    void setView(void *view);

    void setDataSource(const char *url);

  private:

    void putMsg(PlayerMsgType msgType, const PlayerMsgContent &msgContent, bool trigger);

    void processVideoLoop();

    void changePlayerStatus(PlayerStatus newStatus);

    int mainService();

  private:
    std::unique_ptr<PlayerParamsSet> mParamsSet{nullptr};
    std::unique_ptr<IDataSource> mDataSource{nullptr};

    std::unique_ptr<IPlayerMsgProcessor> mMsgProcessor{nullptr};
    std::unique_ptr<PlayerMsgController> mMsgController{nullptr};

    std::atomic_bool mCanceled{false};
    std::atomic_bool mMainServiceCanceled{true};
    std::unique_ptr<SNThread> mThread{nullptr};

    PlayerStatus mOlderStatus{PlayerStatus::IDLE};
    std::atomic<PlayerStatus> mStatus{PlayerStatus::IDLE};

    std::unique_ptr<DemuxerService> mDemuxerService{nullptr};
  };

} // Sivin

#endif //SIVINPLAYER_SIVINPLAYER_H
