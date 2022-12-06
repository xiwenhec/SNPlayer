//
// Created by sivin on 12/6/22.
//

#ifndef DATASOURCETEST_AVFORMATDEMUXER_H
#define DATASOURCETEST_AVFORMATDEMUXER_H

#include <atomic>
#include <utils/SNThread.h>
#include "IDemuxer.h"

extern "C" {
#include <libavformat//avformat.h>
}

namespace Sivin {

    class AVFormatDemuxer : public IDemuxer {

    public:

        explicit AVFormatDemuxer(std::string &path);

        int open();

        int open(AVInputFormat *inputFormat);

    private:

        void init();

        static int interrupt_cb(void *opaque);

        static int readLoop();

        static inline int avio_callback_read(void *arg, uint8_t *buffer, int size);

        static inline int64_t avio_callback_seek(void *arg, int64_t offset, int whence);

    private:
        AVFormatContext *mCtx{nullptr};
        AVIOContext *mIOCtx{nullptr};
        AVDictionary *mInputOpts = nullptr;
        bool bOpened{false};
        std::atomic_bool mInterrupted{false};
        std::shared_ptr<SNThread> mThread{nullptr};
    };
}


#endif //DATASOURCETEST_AVFORMATDEMUXER_H
