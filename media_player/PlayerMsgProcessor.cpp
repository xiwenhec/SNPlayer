//
// Created by sivin on 23-1-14.
//
#include "base/media/SNMediaInfo.h"
#include "data_source/IDataSource.h"
#include "demuxer/DemuxerService.h"
#include <memory>
#include <utility>
#define LOG_TAG "PlayerMsgProcessor"
#include "MediaPlayerDef.h"
#include "PlayerMsgProcessor.h"
#include "data_source/DataSourceFactory.h"
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

    // if (mPlayer.mParams->url.empty() && mPlayer.mBSReadCb == nullptr) {
    //   //URL是空，需要通过mBSReadCb，来代理播放源，如果都没有设置，则表示播放错误
    // }

    if (mPlayer.mPlayStatus != PlayerStatus::INITIALIZED && mPlayer.mPlayStatus != PlayerStatus::STOPPED) {
      SN_LOGD("process parepare msg error, status = %d", mPlayer.mPlayStatus.load());
      return;
    }
    mPlayer.changePlayerStatus(PlayerStatus::PREPARED_INIT);

    //准备DataSource
    if (!mPlayer.mParams->url.empty()) {
      int ret = openUrl();
      if (ret < 0) {
        SN_LOGE("prepare failed. reason: openUrl:%s failed.", mPlayer.mParams->url.c_str());
        return;
      }
    }

    //创建解封装Service
    mPlayer.mDemuxerService = std::make_unique<DemuxerService>(mPlayer.mDataSource);
    if (mPlayer.mSeekPos > 0) {
      mPlayer.mDemuxerService->seek(mPlayer.mSeekPos, 0, -1);
    } else {
      mPlayer.resetSeekStatus();
    }

    int ret = mPlayer.mDemuxerService->initOpen(IDemuxer::DEMUXER_TYPE_BITSTREAM);
    if (ret < 0) {
      SN_LOGE("prepare failed. reason: demuxerService open faild.");
      return;
    }

    int nbStream = mPlayer.mDemuxerService->getNbStreams();
    std::unique_ptr<SNStreamInfo> streamInfo;
    for (int i = 0; i < nbStream; i++) {
      mPlayer.mDemuxerService->getStreamInfo(streamInfo, i);
      if (mPlayer.mDuration < 0) {
        mPlayer.mDuration = streamInfo->duration;
      }
      if (!mPlayer.mParams->disableVideo && streamInfo->type == StreamType::STREAM_TYPE_VIDEO) {
        if (mPlayer.mCurrentVideoIndex < 0) {
          mPlayer.mCurrentVideoIndex = streamInfo->index;
        }
        mPlayer.mMediaInfo.mStreamInfoQueue.push_back(std::move(streamInfo));
      } else if (!mPlayer.mParams->disableAudio && streamInfo->type == StreamType::STREAM_TYPE_AUDIO) {
        if (mPlayer.mCurrentAudioIndex < 0) {
          mPlayer.mCurrentAudioIndex = streamInfo->index;
        }
        mPlayer.mMediaInfo.mStreamInfoQueue.push_back(std::move(streamInfo));
      }
    }

    std::unique_ptr<SNMeidaInfo> snMediaInfo{};
    mPlayer.mDemuxerService->getMediaInfo(snMediaInfo);
    if (snMediaInfo) {
      mPlayer.mMediaInfo.totalBitrate = snMediaInfo->totalBitrate;
    }
    mPlayer.changePlayerStatus(PlayerStatus::PREPARING);
  }


  /*
    打开播放器流地址
  */
  int PlayerMsgProcessor::openUrl() {
    int ret = -1;
    mPlayer.mDataSource = std::shared_ptr<IDataSource>(DataSourceFactory::create(mPlayer.mParams->url));
    if (mPlayer.mDataSource) {
      ret = mPlayer.mDataSource->open(0);
    }
    return ret;
  }


}// namespace Sivin