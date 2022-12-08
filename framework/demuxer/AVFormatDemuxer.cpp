//
// Created by sivin on 12/6/22.
//

#include <utils/SNFFUtils.h>
#include "AVFormatDemuxer.h"
#include "utils/SNTimer.h"
#include "utils/SNLog.h"
#include "utils/SNUtils.h"

extern "C" {
#include <libavutil/intreadwrite.h>
}

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
                    //TODO:Sivin ????
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
        if (bExited) {
            return -1;
        }
        if (bPaused) {
            return 0;
        }
        if (bEOS) {
            std::unique_lock<std::mutex> waitLock(mQueMutex);
            if (bEOS) {
                mQueCond.wait(waitLock, [this]() { return bPaused || mInterrupted || bExited; });
            }
        }
        if (bEOS || bPaused) {
            return 0;
        }

        std::unique_ptr<ISNPacket> pkt{};
        int ret = readPacketInternal(pkt);

        return 0;
    }

    void AVFormatDemuxer::start() {
        bPaused = false;
        mThread->start();
    }

    int AVFormatDemuxer::readPacketInternal(std::unique_ptr<ISNPacket> &packet) {
        if (!bOpened) {
            return -1;
        }

        AVPacket *pkt = av_packet_alloc();
        if (!pkt) {
            return -1;
        }
        int error = 0;
        do {
            error = av_read_frame(mCtx, pkt);
            if (error < 0) {
                if (error == AVERROR_EOF) {
                    if (mCtx->pb && mCtx->pb->error == AVERROR(EAGAIN)) {
                        av_packet_free(&pkt);
                        return mCtx->pb->error;
                    }

                    if (mCtx->pb && mCtx->pb->error < 0) {
                        av_packet_free(&pkt);
                        int ret = mCtx->pb->error;
                        mCtx->pb->error = 0;
                        return ret;
                    }

                    av_packet_free(&pkt);
                    return 0;// EOS
                }

                av_packet_free(&pkt);
                return error;
            }
            //如果当前的流需要被解码，则跳出循环
            if (mStreamCtxMap[pkt->stream_index] && mStreamCtxMap[pkt->stream_index]->opened) {
                break;
            }
            av_packet_unref(pkt);
        } while (true);

        if (pkt->pts == AV_NOPTS_VALUE) {
            NS_LOGW("pkt pts error\n");
        }

        if (pkt->dts == AV_NOPTS_VALUE) {
            NS_LOGW("pkt dts error\n");
        }

        int streamIndex = pkt->stream_index;
        if (!mStreamCtxMap[pkt->stream_index]->bsfInited) {
            createBsf(pkt, streamIndex);
            mStreamCtxMap[pkt->stream_index]->bsfInited = true;
        }

        return 0;
    }

    int AVFormatDemuxer::createBsf(AVPacket *pkt, int index) {
        std::string bsfName{};
        const AVCodecParameters *codecpar = mCtx->streams[index]->codecpar;

        //表示需要annexb的打包格式
        if (mVideoStreamType == BitStreamType::BITSTREAM_TYPE_MERGE) {
            //extradata表示当前NALU码流可能是AVCC的打包格式
            if (codecpar->codec_id == AV_CODEC_ID_H264 && codecpar->extradata != nullptr &&
                (codecpar->extradata[0] == 1)) {
                bsfName = "h264_mp4toannexb";
            } else if (codecpar->codec_id == AV_CODEC_ID_HEVC && codecpar->extradata_size >= 5 &&
                       AV_RB32(codecpar->extradata) != 0x0000001 && AV_RB24(codecpar->extradata) != 0x000001) {
                bsfName = "hevc_mp4toannexb";
            }

        } else if (mVideoStreamType == BITSTREAM_TYPE_EXTRACT) {
            //将码流转换成AVCC的格式
            if (codecpar->codec_id == AV_CODEC_ID_H264 && codecpar->extradata != nullptr &&
                (codecpar->extradata[0] != 1)) {
                //TODO:Sivin 待实现
//                bsfName = "h26xAnnexb2xVcc";
            } else if (codecpar->codec_id == AV_CODEC_ID_HEVC && codecpar->extradata_size >= 5 &&
                       !(AV_RB32(codecpar->extradata) != 0x0000001 &&
                         AV_RB24(codecpar->extradata) != 0x000001)) {
//                bsfName = "h26xAnnexb2xVcc";
            }
        }

        if (!bsfName.empty()) {
            std::lock_guard<std::mutex> lockGuard{mStreamCtxMutex};
            mStreamCtxMap[index]->bsf = std::unique_ptr<IAVBSF>(AVBSFFactory::create(bsfName));
            int ret = mStreamCtxMap[index]->bsf->init(bsfName, mCtx->streams[index]->codecpar);
            if (ret < 0) {
                NS_LOGE("create %s bsf error \n",bsfName.c_str());
                return ret;
            }
        }
        return 0;
    }


}
