//
// Created by Sivin on 2022-11-26.
//

#ifndef SIVINPLAYER_SIVINPLAYER_H
#define SIVINPLAYER_SIVINPLAYER_H

#include "IMediaPlayer.h"
#include "data_source/IDataSource.h"
#include "utils/SNThread.h"
#include "demuxer/DemuxerService.h"
#include "PlayerParams.h"
#include "MediaPlayerDef.h"
#include "PlayerMsgController.h"
#include "memory"


namespace Sivin {

    class SivinPlayer : public IMediaPlayer {
        friend class PlayerMsgProcessor;

    public:
        SivinPlayer();

        ~SivinPlayer();

    public:
        virtual void setView(void *view) override;

        virtual void setDataSource(const char *url) override;

        virtual void prepare() override;

        virtual void start() override;

    private:
        void putMsg(PlayerMsgType msgType, const PlayerMsg &msgContent, bool trigger);

        void processVideoLoop();

        void changePlayerStatus(PlayerStatus newStatus);

        int mainService();

    private:
        std::unique_ptr<PlayerParams> mParams{nullptr};
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

} // namespace Sivin

#endif // SIVINPLAYER_SIVINPLAYER_H
