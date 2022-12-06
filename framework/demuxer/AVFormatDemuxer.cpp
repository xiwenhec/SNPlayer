//
// Created by sivin on 12/6/22.
//

#include <utils/SNFFUtils.h>
#include "AVFormatDemuxer.h"
#include "utils/SNTimer.h"
#include "utils/SNLog.h"
#include "utils/SNUtils.h"

namespace Sivin {

    const static int INITIAL_BUFFER_SIZE = 1024 * 32;

    AVFormatDemuxer::AVFormatDemuxer(std::string &path) : IDemuxer(path) {
        init();
        NS_TRACE;
    }


    void AVFormatDemuxer::init() {
        mCtx = avformat_alloc_context();
        //当打开网络流出现阻塞时会回到这个函数，用于判断是否中断
        mCtx->interrupt_callback.callback = interrupt_cb;
        mCtx->interrupt_callback.opaque = this;
        mCtx->correct_ts_overflow = 0; //TODO:Sivin 这个值的含义是？
        mThread = MAKE_UNIQUE_THREAD(readLoop, "avForamtDemuxer");
    }


    int AVFormatDemuxer::open() {
        return open(nullptr);
    }

    //TODO:Sivin 错误处理
    int AVFormatDemuxer::open(AVInputFormat *inputFormat) {
        if (bOpened) { return 0; }

        int64_t startTime = SNTimer::getSteadyTimeMs();
        //是否直接使用文件路径
        bool useFileName = false;
        if (mReadCb != nullptr) {
            auto *readBuffer = static_cast<uint8_t *>(av_malloc(INITIAL_BUFFER_SIZE));
            mIOCtx = avio_alloc_context(readBuffer, INITIAL_BUFFER_SIZE, 0, this,
                                        mReadCb ? avio_callback_read : nullptr,
                                        nullptr, mSeekCb ? avio_callback_seek : nullptr);
            if (mIOCtx == nullptr) {
                av_free(mIOCtx);
                return -1;
            }
            mCtx->pb = mIOCtx;
        } else {
            useFileName = true;
        }

        av_dict_set_int(&mInputOpts, "safe", 0, 0);
        av_dict_set(&mInputOpts, "protocol_whitelist", "file,http,https,tcp,tls", 0);
        av_dict_set_int(&mInputOpts, "usetoc", 1, 0);
        av_dict_set_int(&mInputOpts, "flv_strict_header", 1, 0);

        const char *filename = mPath.c_str();
        if (!useFileName) {
            if (SNUtils::startWith(mPath, {"http://", "https://"})) {
                const AVInputFormat *mp4Format = av_find_input_format("mp4");
                if (mp4Format && av_match_ext(filename, mp4Format->extensions)) {
                    filename = "http://xxx";
                }
            }
        }

        int ret = avformat_open_input(&mCtx, filename, inputFormat, mInputOpts ? &mInputOpts : nullptr);

        if (ret < 0) {
            NS_LOGE("avformat_open_input error %d,%s,", ret, SNFFUtils::getErrorString(ret));
            if (ret == AVERROR_PROTOCOL_NOT_FOUND) {
                return -1;
            }
            if (ret == AVERROR_EXIT || mInterrupted) {
                return -1;
            }
            return ret;
        }

        if (strcmp(mCtx->iformat->name, "mov,mp4,m4a,3gp,3g2,mj2") != 0) {
            mCtx->fps_probe_size = 0;
        } else {
            mCtx->fps_probe_size = 5;
        }

        ret = avformat_find_stream_info(mCtx, nullptr);

        if (mInterrupted) {
            NS_LOGD("interrupted\n");
            return -1;
        }

        if (ret < 0 && ret != AVERROR_EOF) {
            NS_LOGE("avformat_find_stream_info error %d:%s\n", ret, SNFFUtils::getErrorString(ret));
            return ret;
        }
        bOpened = true;
        return 0;
    }

    int AVFormatDemuxer::avio_callback_read(void *arg, uint8_t *buffer, int size) {
        auto *demuxer = static_cast<AVFormatDemuxer *>(arg);
        int ret = demuxer->mReadCb(demuxer->mUserArg, buffer, size);
        return ret ? ret : AVERROR_EOF;
    }

    int64_t AVFormatDemuxer::avio_callback_seek(void *arg, int64_t offset, int whence) {
        auto *demuxer = static_cast<AVFormatDemuxer *>(arg);
        return demuxer->mSeekCb(demuxer->mUserArg, offset, whence);
    }

    int AVFormatDemuxer::interrupt_cb(void *opaque) {
        return static_cast<AVFormatDemuxer *>(opaque)->mInterrupted;
    }

    int AVFormatDemuxer::readLoop() {

        return 0;
    }


}
