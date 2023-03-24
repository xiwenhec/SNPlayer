//
// Created by Sivin on 2022-11-26.
//

#ifndef SIVINPLAYER_SIVINPLAYER_H
#define SIVINPLAYER_SIVINPLAYER_H

#include "IMediaPlayer.h"
#include "MediaPlayerDef.h"
#include "PlayerMsgController.h"
#include "PlayerParams.h"
#include "data_source/IDataSource.h"
#include "demuxer/DemuxerService.h"
#include "utils/SNThread.h"
#include <mutex>


namespace Sivin {

  class SnPlayer : public IMediaPlayer {
    friend class PlayerMsgProcessor;

  public:
    SnPlayer();

    ~SnPlayer();

  public:
    virtual std::string getName() override {
      return "SNPlayer";
    };

    virtual void setView(void *view) override;

    virtual void setDataSource(const char *url) override;

    virtual void prepare() override;

    virtual void start() override;

    virtual void pause() override;

    virtual void seekTo(int64_t seekPos, bool isAccurate) override;

    virtual void stop() override;
    

  private:
    void putMsg(PlayerMsgType msgType, const PlayerMsg &msgContent, bool trigger);

    void processVideoLoop();

    void changePlayerStatus(PlayerStatus newStatus);

    int mainService();

    void resetSeekStatus();

  private:
    //用于记录播放的各种参数，比如播放地址，播放速度等
    std::unique_ptr<PlayerParams> mParams{nullptr};
    std::shared_ptr<IDataSource> mDataSource{nullptr};

    std::mutex mPlayerMutex;

    std::unique_ptr<IPlayerMsgProcessor> mMsgProcessor{nullptr};
    std::unique_ptr<PlayerMsgController> mMsgController{nullptr};

    std::atomic_bool mCanceled{false};
    std::atomic_bool mMainServiceCanceled{true};
    std::unique_ptr<SNThread> mPlayerThread{nullptr};

    PlayerStatus mOlderStatus{PlayerStatus::IDLE};
    std::atomic<PlayerStatus> mPlayStatus{PlayerStatus::IDLE};

    std::unique_ptr<DemuxerService> mDemuxerService{nullptr};

    std::atomic<int64_t> mSeekPos{-1};
  };

}// namespace Sivin

#endif// SIVINPLAYER_SIVINPLAYER_H
