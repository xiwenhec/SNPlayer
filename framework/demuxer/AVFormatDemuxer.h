//
// Created by sivin on 12/6/22.
//

#ifndef DATASOURCETEST_AVFORMATDEMUXER_H
#define DATASOURCETEST_AVFORMATDEMUXER_H

#include <atomic>
#include <utils/SNThread.h>
#include <base/media/ISNPacket.h>
#include "IDemuxer.h"
#include <map>
#include "base/media/IAVBSF.h"
#include <deque>

extern "C" {
#include <libavformat//avformat.h>
}

namespace Sivin {

    class AVFormatDemuxer : public IDemuxer {
    private:
        class AVStreamCtx {
        public:
            std::unique_ptr<IAVBSF> bsf;
            bool bsfInited{false};
            bool opened{true};
        };

    public:
        explicit AVFormatDemuxer(std::string &path);

        ~AVFormatDemuxer() override;

        int open() override;

        int open(AVInputFormat *inputFormat);

        void start() override;

        void preStop() override;

        void stop() override;

        int readPacket(std::unique_ptr<ISNPacket> &packet, int index) override;

        int openStream(int index) override;

        void closeStream(int index) override;

    private:

        void init();

        int readLoop();

        int readPacketInternal(std::unique_ptr<ISNPacket> &packet);

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
        std::deque<std::unique_ptr<ISNPacket>> mPacketQueue{};

        std::mutex mStreamCtxMutex{};
        //key为流index
        std::map<int, std::unique_ptr<AVStreamCtx>> mStreamCtxMap{};

        std::atomic<int64_t> mError{0};
    };
}


#endif //DATASOURCETEST_AVFORMATDEMUXER_H
