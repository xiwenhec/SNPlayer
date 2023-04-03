//
// Created by Sivin on 2022-11-26.
//

#ifndef SIVINPLAYER_SIVINPLAYER_H
#define SIVINPLAYER_SIVINPLAYER_H

#include "BufferController.h"
#include "DeviceManager.h"
#include "IMediaPlayer.h"
#include "MediaPlayerDef.h"
#include "PlayerError.h"
#include "PlayerMsgController.h"
#include "PlayerNotifier.h"
#include "PlayerParams.h"
#include "base/SNRet.h"
#include "base/media/SNFrame.h"
#include "base/media/SNMediaInfo.h"
#include "base/media/SNPacket.h"
#include "data_source/IDataSource.h"
#include "demuxer/DemuxerService.h"
#include "utils/SNThread.h"
#include "PlayerStatistic.h"
#include "ReferClock.h"
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>

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

    virtual void setListener(const PlayerListener &listener) override;

    virtual void prepare() override;

    virtual void start() override;

    virtual void pause() override;

    virtual void seekTo(int64_t seekPos, bool isAccurate) override;

    virtual void stop() override;


  private:
    void putMsg(PlayerMsgType msgType, const PlayerMsg &msgContent, bool trigger);

    void processMediaLoop();

    void changePlayerStatus(PlayerStatus newStatus);

    int mainService();

    void resetSeekStatus();

    void notifyError(PlayerError error);

    bool isSeeking() {
      return -1 != mSeekPos;
    }

    int64_t getCurrentPosition();

  private:
    int64_t getPlayerBufferDuration(bool gotMax);

    void doReadPacket();

    SNRet readPacket();

    void doDecode();

    int fillVideoFrame();

    int64_t decodeVideoPacket(std::unique_ptr<SNPacket> &packet);
    int64_t decodeAudioPacket(std::unique_ptr<SNPacket> &packet);

    int doRender();
    bool renderVideo(bool forceRender);
    int sendVideoFrameToRender(std::unique_ptr<SNFrame> frame);

    //待实现
    void closeVideo();
    void closeAudio();

    void setupAVDevice();

    void setupVideoDevice();

  private:
    //用于记录播放的各种参数，比如播放地址，播放速度等
    std::unique_ptr<PlayerParams> mParams{nullptr};
    std::shared_ptr<IDataSource> mDataSource{nullptr};

    PlayerListener mListener{};

    std::mutex mPlayerMutex;

    std::unique_ptr<PlayerStatistic> mStat{nullptr};
    std::unique_ptr<IPlayerMsgProcessor> mMsgProcessor{nullptr};
    std::unique_ptr<PlayerMsgController> mMsgController{nullptr};
    std::unique_ptr<BufferController> mBufferController{nullptr};
    std::unique_ptr<DeviceManager> mDeviceManager{nullptr};
    std::unique_ptr<PlayerNotifier> mNotifier{nullptr};

    //存放解码后的帧序列
    std::queue<std::unique_ptr<SNFrame>> mVideoFrameQue{};
    std::deque<std::unique_ptr<SNFrame>> mAudioFrameQue{};
    std::unique_ptr<SNPacket> mVideoPacket{};
    std::unique_ptr<SNPacket> mAudioPacket{};

    std::atomic_bool mCanceled{false};
    std::atomic_bool mMainServiceCanceled{true};
    std::unique_ptr<SNThread> mPlayerThread{nullptr};

    PlayerStatus mOlderStatus{PlayerStatus::IDLE};
    std::atomic<PlayerStatus> mPlayStatus{PlayerStatus::IDLE};

    std::unique_ptr<DemuxerService> mDemuxerService{nullptr};

    //用户有seek发生，处理完成后，将会重置为SN_UNKNOWN_VALUE
    std::atomic<int64_t> mSeekPos{-1};
    bool mSeekFlag{false};//TODO:need rename
    std::atomic<int64_t> mCurrentPos{0};
    int64_t mDuration{-1};

    int mCurrentVideoIndex{-1};
    int mCurrentAudioIndex{-1};

    //TODO:这个变量的存在似乎多余
    bool mHaveVideoPkt{false};
    bool mHaveAudioPkt{false};

    int mVideoWidth{0};
    int mVideoHeight{0};

    MediaInfo mMediaInfo{};

    bool mReadEos{false};
    bool mVideoDecoderEOS{false};
    bool mAudioDecoderEOS{false};
    bool mBufferIsFull{false};

    bool mCalculateSpeedUsePacket{false};

    //出现内存报警
    bool mLowMem{false};

    //移动app当前处于前台还是后台
    std::atomic<AppStatus> mAppStatus{AppStatus::FOREGROUND};
    ReferClock mMasterClock;
  };

}// namespace Sivin

#endif// SIVINPLAYER_SIVINPLAYER_H
