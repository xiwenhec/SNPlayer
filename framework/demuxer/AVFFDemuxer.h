//
// Created by sivin on 12/6/22.
//

#ifndef DATASOURCETEST_AVFORMATDEMUXER_H
#define DATASOURCETEST_AVFORMATDEMUXER_H

#include "base/media/SNMediaInfo.h"

extern "C" {
#include <libavformat//avformat.h>
}

#include "IDemuxer.h"
#include <map>
#include <deque>
#include <atomic>
#include <utils/SNThread.h>
#include <base/media/SNPacket.h>
#include "base/media/IAVBSF.h"

namespace Sivin {

  class AVFFDemuxer : public IDemuxer {

  private:
    class AVStreamCtx {
    public:
      std::unique_ptr<IAVBSF> bsf;
      bool bsfInited{false};
      bool opened{true};
    };

  public:
    explicit AVFFDemuxer(std::string path);

    ~AVFFDemuxer() override;

    int open() override;

    int open(AVInputFormat *inputFormat);

    void start() override;

    void preStop() override;

    void stop() override;

    int readPacket(std::unique_ptr<SNPacket> &packet, int index) override;

    int openStream(int index) override;

    void closeStream(int index) override;


  public:
    int getMediaInfo(std::unique_ptr<SNMeidaInfo> &mediaInfo) override;
    int getNbStreams() const override;
    int getStreamInfo(std::unique_ptr<SNStreamInfo> &streamInfo, int index) override;

  private:
    void init();

    int readLoop();

    int readPacketInternal(std::unique_ptr<SNPacket> &packet);

    static int interrupt_cb(void *opaque);

    static inline int avio_callback_read(void *arg, uint8_t *buffer, int size);

    static inline int64_t avio_callback_seek(void *arg, int64_t offset, int whence);

    int createBsf(AVPacket *pkt, int index);

  private:
    AVFormatContext *mCtx{nullptr};
    AVIOContext *mIOCtx{nullptr};
    AVDictionary *mInputOpts = nullptr;
    bool bOpened{false};
    bool bPaused{false};
    bool bExited{false};
    bool bEOS{false};
    std::atomic_bool mInterrupted{false};
    std::shared_ptr<SNThread> mThread{nullptr};

    std::mutex mMutex{};
    std::mutex mQueMutex{};
    std::condition_variable mQueCond{};
    
    int MAX_QUEUE_SIZE = 60;
    //这个packet里存放着音频和视频的packet
    std::deque<std::unique_ptr<SNPacket>> mPacketQueue{};

    std::mutex mStreamCtxMutex{};
    //key为流index
    std::map<int, std::unique_ptr<AVStreamCtx>> mStreamCtxMap{};

    std::atomic<int64_t> mError{0};
  };
}// namespace Sivin


#endif//DATASOURCETEST_AVFORMATDEMUXER_H
