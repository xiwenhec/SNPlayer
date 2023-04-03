//
// Created by sivin on 12/6/22.
//

#define LOG_TAG "AVFFDemuxer"

extern "C" {
#include <libavutil/intreadwrite.h>
}

#include "base/media/SNMediaInfo.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <memory>
#include <utils/SNFFUtil.h>
#include "AVFFDemuxer.h"
#include "utils/SNTimer.h"
#include "utils/SNLog.h"
#include "utils/SNUtils.h"
#include "base/media/SNAVPacket.h"

#define ADD_LOCK \
  std::unique_lock<std::mutex> lock { mMutex }

#define CHECK_CTX             \
  if (mCtx == nullptr) {      \
    SN_TRACE;                 \
    SN_LOGE("mCtx is null."); \
    return -1;                \
  }

namespace Sivin {

  const static int INITIAL_BUFFER_SIZE = 1024 * 32;

  AVFFDemuxer::AVFFDemuxer(std::string path) : IDemuxer(std::move(path)) {
    init();
    SN_TRACE;
  }

  void AVFFDemuxer::init() {
    mCtx = avformat_alloc_context();
    //当打开网络流出现阻塞时会回到这个函数，用于判断是否中断
    mCtx->interrupt_callback.callback = interrupt_cb;
    mCtx->interrupt_callback.opaque = this;
    mReadThread = MAKE_UNIQUE_THREAD(readLoop, "avForamtDemuxer");
  }


  AVFFDemuxer::~AVFFDemuxer() {
    stop();
    if (mCtx) {
      avformat_close_input(&mCtx);
      mCtx = nullptr;
    }
    if (mIOCtx) {
      av_free(mIOCtx->buffer);
      av_free(mIOCtx);
      mIOCtx = nullptr;
    }

    mStreamCtxMap.clear();
    mPacketQueue.clear();
    mOpened = false;

    if (mInputOpts) {
      av_dict_free(&mInputOpts);
      mInputOpts = nullptr;
    }
  }


  //TODO:Sivin 错误处理
  int AVFFDemuxer::open() {
    if (mOpened) {
      return 0;
    }
    SN_TRACE;
    int64_t costTime = SNTimer::getSteadyTimeMs();
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

    int ret = avformat_open_input(&mCtx, filename, nullptr, mInputOpts ? &mInputOpts : nullptr);
    if (ret < 0) {
      SN_LOGE("avformat_open_input error %d, %s", ret, SNFFUtil::getErrorString(ret));
      if (ret == AVERROR_PROTOCOL_NOT_FOUND) {
        return -1;
      }
      if (ret == AVERROR_EXIT || mInterrupted) {
        return -1;
      }
      return ret;
    }
    costTime = SNTimer::getSteadyTimeMs() - costTime;
    SN_LOGI("avformat_open_input success. cost time = %ldms", costTime);

    if (strcmp(mCtx->iformat->name, "mov,mp4,m4a,3gp,3g2,mj2") != 0) {
      mCtx->fps_probe_size = 0;
    } else {
      mCtx->fps_probe_size = 5;
    }

    ret = avformat_find_stream_info(mCtx, nullptr);

    if (mInterrupted) {
      SN_LOGD("interrupted");
      return -1;
    }

    if (ret < 0 && ret != AVERROR_EOF) {
      SN_LOGE("avformat_find_stream_info error %d:%s", ret, SNFFUtil::getErrorString(ret));
      return ret;
    }
    mOpened = true;
    return 0;
  }

  int AVFFDemuxer::avio_callback_read(void *arg, uint8_t *buffer, int size) {
    auto *demuxer = static_cast<AVFFDemuxer *>(arg);
    int ret = demuxer->mReadCb(demuxer->mUserArg, buffer, size);
    return ret ? ret : AVERROR_EOF;
  }

  int64_t AVFFDemuxer::avio_callback_seek(void *arg, int64_t offset, int whence) {
    auto *demuxer = static_cast<AVFFDemuxer *>(arg);
    return demuxer->mSeekCb(demuxer->mUserArg, offset, whence);
  }

  int AVFFDemuxer::interrupt_cb(void *opaque) {
    return static_cast<AVFFDemuxer *>(opaque)->mInterrupted;
  }

  int AVFFDemuxer::readLoop() {
    if (mExited) {
      return -1;
    }
    if (bPaused) {
      return 0;
    }
    if (mReadEOS) {
      std::unique_lock<std::mutex> waitLock(mQueMutex);
      if (mReadEOS) {
        mQueCond.wait(waitLock, [this]() { return bPaused || mInterrupted || mExited; });
      }
    }
    if (mReadEOS || bPaused) {
      return 0;
    }

    std::unique_ptr<SNPacket> pkt{};

    auto ret = readPacketInternal(pkt);
    if (ret == SNRet::Status::SUCCESS) {
      std::unique_lock<std::mutex> waitLock{mQueMutex};
      if (mPacketQueue.size() > MAX_QUEUE_SIZE) {
        SN_LOGD("read packet thread wait..");
        mQueCond.wait(waitLock, [this]() {
          return mPacketQueue.size() <= MAX_QUEUE_SIZE || bPaused || mInterrupted || mExited;
        });
        SN_LOGD("read packet thread wakeup");
      }
      mPacketQueue.push_back(std::move(pkt));

    } else if (ret == SNRet::Status::AGAIN) {
      mReadEOS = true;

    } else {
      mError = -1;
      std::unique_lock<std::mutex> waitLock(mQueMutex);
      mQueCond.wait_for(waitLock, std::chrono::milliseconds(10),
          [this]() { return bPaused || mInterrupted || mExited; });
    }
    return 0;
  }

  void AVFFDemuxer::start() {
    bPaused = false;
    mReadThread->start();
  }


  void AVFFDemuxer::preStop() {
    {
      std::unique_lock<std::mutex> waitLock(mQueMutex);
      mExited = true;
    }
    mQueCond.notify_one();
  }


  void AVFFDemuxer::stop() {
    {
      std::unique_lock<std::mutex> waitLock(mQueMutex);
      bPaused = true;
    }
    mQueCond.notify_one();

    if (mReadThread) {
      mReadThread->stop();
    }
  }


  SNRet AVFFDemuxer::readPacket(std::unique_ptr<SNPacket> &packet, int index) {
    if (mReadThread->getStatus() == SNThread::Status::IDLE) {
      SN_LOGW("read packet thread not start, will read from Internal");
      return readPacketInternal(packet);
    } else {
      std::unique_lock<std::mutex> waitLock(mQueMutex);
      if (!mPacketQueue.empty()) {
        packet = std::move(mPacketQueue.front());
        mPacketQueue.pop_front();
        mQueCond.notify_one();
        return {SNRet::Status::SUCCESS, static_cast<int>(packet->getSize())};
      }
      if (mReadEOS) {
        return SNRet::Status::EOS;
      }
      if (mError < 0) {
        return SNRet::Status::ERROR;
      }
      return SNRet::Status::AGAIN;
    }
  }


  SNRet AVFFDemuxer::readPacketInternal(std::unique_ptr<SNPacket> &packet) {
    if (!mOpened) {
      return SNRet::Status::ERROR;
    }
    AVPacket *pkt = av_packet_alloc();
    if (!pkt) {
      return SNRet::Status::ERROR;
    }
    int error = 0;
    do {
      error = av_read_frame(mCtx, pkt);
      if (error < 0) {
        if (error == AVERROR_EOF) {
          if (mCtx->pb && mCtx->pb->error == AVERROR(EAGAIN)) {
            av_packet_free(&pkt);
            return SNRet::Status::ERROR;
          }

          if (mCtx->pb && mCtx->pb->error < 0) {
            av_packet_free(&pkt);
            int ret = mCtx->pb->error;
            mCtx->pb->error = 0;
            return SNRet::Status::ERROR;
          }
          av_packet_free(&pkt);
          return SNRet::Status::EOS;// EOS
        }
        av_packet_free(&pkt);
        return SNRet::Status::ERROR;
      }

      //如果当前的流需要被解码，则跳出循环
      if (mStreamCtxMap[pkt->stream_index] && mStreamCtxMap[pkt->stream_index]->opened) {
        break;
      }
      if (pkt->stream_index == 0) {
        SN_LOGI("get packet pts:%d",
            av_rescale_q(pkt->pts, mCtx->streams[pkt->stream_index]->time_base, av_get_time_base_q()));
      }
      av_packet_unref(pkt);
    } while (true);

    if (pkt->pts == AV_NOPTS_VALUE) {
      SN_LOGW("pkt pts error");
    }

    if (pkt->dts == AV_NOPTS_VALUE) {
      SN_LOGW("pkt dts error");
    }

    int streamIndex = pkt->stream_index;
    if (!mStreamCtxMap[pkt->stream_index]->bsfInited) {
      createBsf(pkt, streamIndex);
      mStreamCtxMap[pkt->stream_index]->bsfInited = true;
    }

    bool needUpdateExtraData = false;
    size_t newExtraDataSize = 0;
    const uint8_t *newExtraData = av_packet_get_side_data(pkt, AV_PKT_DATA_NEW_EXTRADATA, &newExtraDataSize);
    if (newExtraData) {
      AVCodecParameters *codecpar = mCtx->streams[streamIndex]->codecpar;
      av_free(codecpar->extradata);
      codecpar->extradata = static_cast<uint8_t *>(av_malloc(newExtraDataSize + AV_INPUT_BUFFER_PADDING_SIZE));
      memcpy(codecpar->extradata, newExtraData, newExtraDataSize);
      codecpar->extradata_size = newExtraDataSize;
      createBsf(pkt, streamIndex);
      needUpdateExtraData = true;
    }

    if (mStreamCtxMap[pkt->stream_index]->bsf) {
      int index = pkt->stream_index;
      mStreamCtxMap[index]->bsf->sendPacket(pkt);
      int ret = mStreamCtxMap[index]->bsf->receivePacket(pkt);
      if (ret < 0) {
        av_packet_free(&pkt);
        return SNRet::Status::ERROR;
      }
    }

    //转换成微妙
    if (pkt->pts != AV_NOPTS_VALUE) {
      pkt->pts = av_rescale_q(pkt->pts, mCtx->streams[pkt->stream_index]->time_base, av_get_time_base_q());
    }

    if (pkt->dts != AV_NOPTS_VALUE) {
      pkt->dts = av_rescale_q(pkt->dts, mCtx->streams[pkt->stream_index]->time_base, av_get_time_base_q());
    }

    if (pkt->duration > 0) {
      pkt->duration = av_rescale_q(pkt->duration, mCtx->streams[pkt->stream_index]->time_base,
          av_get_time_base_q());
    } else if (mCtx->streams[pkt->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      AVCodecParameters *codecpar = mCtx->streams[pkt->stream_index]->codecpar;
      if (codecpar->sample_rate > 0 && codecpar->frame_size > 0) {
        pkt->duration = codecpar->frame_size * 1000000 / codecpar->sample_rate;
      }
    }

    int packet_size = pkt->size;
    packet = std::unique_ptr<SNPacket>(new SNAVPacket(&pkt));

    if (packet->getInfo().pts != INT64_MIN) {
      if (mCtx->start_time == INT64_MIN) {
        mCtx->start_time = packet->getInfo().pts;
      }
      packet->getInfo().timePosition = packet->getInfo().pts - mCtx->start_time;
    }

    return {SNRet::Status::SUCCESS, packet_size};
  }

  int AVFFDemuxer::createBsf(AVPacket *pkt, int index) {
    std::string bsfName{};
    const AVCodecParameters *codecpar = mCtx->streams[index]->codecpar;

    //表示需要Annexb的打包格式
    if (mVideoStreamType == BitStreamType::MERGE) {
      //extradata里有数据表示，当前是AVCC的打包格式
      if (codecpar->codec_id == AV_CODEC_ID_H264 && codecpar->extradata != nullptr &&
          (codecpar->extradata[0] == 1)) {
        bsfName = "h264_mp4toannexb";
      } else if (codecpar->codec_id == AV_CODEC_ID_HEVC && codecpar->extradata_size >= 5 &&
                 AV_RB32(codecpar->extradata) != 0x0000001 && AV_RB24(codecpar->extradata) != 0x000001) {
        bsfName = "hevc_mp4toannexb";
      }
    } else if (mVideoStreamType == BitStreamType::EXTRACT) {
      //将码流转换成AVCC的格式
      if (codecpar->codec_id == AV_CODEC_ID_H264 && codecpar->extradata != nullptr &&
          (codecpar->extradata[0] != 1)) {
        //TODO:Sivin 待实现
        //bsfName = "h26xAnnexb2xVcc";
      } else if (codecpar->codec_id == AV_CODEC_ID_HEVC && codecpar->extradata_size >= 5 &&
                 !(AV_RB32(codecpar->extradata) != 0x0000001 &&
                     AV_RB24(codecpar->extradata) != 0x000001)) {
        //bsfName = "h26xAnnexb2xVcc";
      }
    }

    if (!bsfName.empty()) {
      std::lock_guard<std::mutex> lockGuard{mStreamCtxMutex};
      mStreamCtxMap[index]->bsf = AVBSFFactory::create(bsfName);
      int ret = mStreamCtxMap[index]->bsf->init(bsfName, mCtx->streams[index]->codecpar);
      if (ret < 0) {
        SN_LOGE("create %s bsf error", bsfName.c_str());
        return ret;
      }
    }
    return 0;
  }


  int AVFFDemuxer::openStream(int index) {
    ADD_LOCK;
    if (index >= mCtx->nb_streams) {
      SN_LOGE("no such stream");
      return -1;
    }
    if (mStreamCtxMap[index] != nullptr) {
      mStreamCtxMap[index]->opened = true;
      return 0;
    }
    mStreamCtxMap[index] = std::make_unique<AVStreamCtx>();
    mStreamCtxMap[index]->opened = true;
    mStreamCtxMap[index]->bsfInited = false;
    return 0;
  }

  void AVFFDemuxer::closeStream(int index) {
    ADD_LOCK;
    if (mStreamCtxMap.find(index) == mStreamCtxMap.end()) {
      SN_LOGI("not opened");
      return;
    }
    mStreamCtxMap[index]->opened = false;
  }

  int AVFFDemuxer::getMediaInfo(std::unique_ptr<SNMeidaInfo> &mediaInfo) {
    CHECK_CTX;
    mediaInfo = std::make_unique<SNMeidaInfo>();
    mediaInfo->totalBitrate = mCtx->bit_rate;
    return 0;
  }

  int AVFFDemuxer::getNbStreams() const {
    CHECK_CTX;
    return mCtx->nb_streams;
  }

  int AVFFDemuxer::getStreamInfo(std::unique_ptr<SNStreamInfo> &streamInfo, int index) {
    ADD_LOCK;
    if (index < 0 || mCtx == nullptr || index > mCtx->nb_streams) {
      return -1;
    }
    SNStreamInfo *info = new SNStreamInfo();
    SNFFUtil::getStreamInfo(mCtx->streams[index], info);
    if (mCtx->duration != AV_NOPTS_VALUE) {
      info->duration = mCtx->duration;
    } else {
      info->duration = 0;
    }
    info->index = index;
    streamInfo = std::unique_ptr<SNStreamInfo>(info);
    return 0;
  }


}// namespace Sivin
