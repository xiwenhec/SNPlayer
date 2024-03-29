extern "C" {
#include <libavcodec/avcodec.h>
}

#include "SNFFDecoder.h"
#include "base/SNRet.h"
#include "base/media/SNPacket.h"
#include "codec/IDecoder.h"
#include "codec/SNActiveDecoder.h"
#include "utils/SNFFUtil.h"
#include "utils/SNLog.h"
#include "base/media/SNAVPacket.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include "base/media/SNAVFrame.h"


namespace Sivin {

  SNFFDecoder::SNFFDecoder() : SNActiveDecoder() {
    name = "ff.decoder";
    mDecoder = std::make_unique<InternalDecoder>();
  }

  SNFFDecoder::~SNFFDecoder() {
    closeDecoder();
  }

  void SNFFDecoder::closeDecoder() {
    if (mDecoder) {
      if (mDecoder->codecCtx) {
        avcodec_close(mDecoder->codecCtx);
        avcodec_free_context(&mDecoder->codecCtx);
        mDecoder->codecCtx = nullptr;
      }
      mDecoder->codec = nullptr;
      if (mDecoder->avFrame) {
        av_frame_free(&mDecoder->avFrame);
        mDecoder->avFrame = nullptr;
      }
    }
  }

  SNRet SNFFDecoder::initDecoder(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags) {
    enum AVCodecID codecId = SNFFUtil::SNCodecToAVCodec(streamInfo->codeId);
    mDecoder->codec = avcodec_find_decoder(codecId);

    if (mDecoder->codec == nullptr) {
      SN_LOGE("couldn't find codec:%d", codecId);
      return SNRet::Status::ERROR;
    }

    mDecoder->codecCtx = avcodec_alloc_context3(mDecoder->codec);
    if (mDecoder->codecCtx == nullptr) {
      SN_LOGE("codecCtx alloc failed");
      return SNRet::Status::ERROR;
    }

    bool isAudio = streamInfo->channels > 0;

    if (isAudio) {
      mDecoder->codecCtx->ch_layout.nb_channels = streamInfo->channels;
      mDecoder->codecCtx->sample_rate = streamInfo->sampleRate;
    }

    if (streamInfo->extraData != nullptr && streamInfo->extraDataSize > 0) {
      mDecoder->codecCtx->extradata = (uint8_t *) av_mallocz(streamInfo->extraDataSize + AV_INPUT_BUFFER_PADDING_SIZE);
      mDecoder->codecCtx->extradata_size = streamInfo->extraDataSize;
    }
    mDecoder->codecCtx->thread_count = 1;
    if (avcodec_open2(mDecoder->codecCtx, mDecoder->codec, nullptr) < 0) {
      SN_LOGE("couldn't open codec");
      avcodec_free_context(&mDecoder->codecCtx);
      return SNRet::Status::ERROR;
    }

    mDecoder->avFrame = av_frame_alloc();

    if (!isAudio) {
      mDecoder->videoInfo.width = mDecoder->codecCtx->width;
      mDecoder->videoInfo.height = mDecoder->codecCtx->height;
      mDecoder->videoInfo.pix_fmt = mDecoder->codecCtx->pix_fmt;
    }
    return SNRet::Status::SUCCESS;
  }

  SNRet SNFFDecoder::enqueueDecoder(std::unique_ptr<SNPacket> &packet) {
    AVPacket *pkt = nullptr;
    if (packet) {
      auto *avPacket = dynamic_cast<SNAVPacket *>(packet.get());
      assert(avPacket);
      pkt = avPacket->toFFPacket();
      pkt->pts = avPacket->getInfo().pts;
      pkt->dts = avPacket->getInfo().dts;
      assert(pkt != nullptr);
    }
    if (pkt == nullptr) {
      SN_LOGW("send a empty pkt to decoder.");
    }
    int ret = avcodec_send_packet(mDecoder->codecCtx, pkt);
    if (ret == 0) {
      packet = nullptr;
    } else if (ret == AVERROR_EOF) {
      return SNRet::Status::EOS;
    } else if (ret == AVERROR(EAGAIN)) {
      return SNRet::Status::AGAIN;
    } else {
      SN_LOGE("decode av paceket error:%d:%s", ret, SNFFUtil::getErrorString(ret));
      return SNRet::Status::ERROR;
    }
    return SNRet::Status::SUCCESS;
  }

  SNRet SNFFDecoder::dequeueDecoder(std::unique_ptr<SNFrame> &frame) {
    int ret = avcodec_receive_frame(mDecoder->codecCtx, mDecoder->avFrame);

    if (ret < 0) {
      if (ret == AVERROR_EOF) {
        return SNRet::Status::EOS;
      } else if (ret == AVERROR(EAGAIN)) {
        return SNRet::Status::AGAIN;
      }
      return SNRet::Status::ERROR;
    }

    if (mDecoder->avFrame->decode_error_flags || mDecoder->avFrame->flags) {
      SN_LOGW("get a error frame\n");
      return SNRet::Status::AGAIN;
    }

    int64_t timePosition = INT64_MIN;
    int64_t utcTime = INT64_MIN;
    if (mDecoder->avFrame->metadata) {
      AVDictionaryEntry *t = av_dict_get(mDecoder->avFrame->metadata, "timePosition", nullptr, AV_DICT_IGNORE_SUFFIX);
      if (t) {
        timePosition = atoll(t->value);
      }
      AVDictionaryEntry *utcEntry = av_dict_get(mDecoder->avFrame->metadata, "utcTime", nullptr, AV_DICT_IGNORE_SUFFIX);
      if (utcEntry) {
        utcTime = atoll(utcEntry->value);
      }
    }
    frame = std::unique_ptr<SNFrame>(new SNAVFrame(mDecoder->avFrame));
    frame->getInfo().timePosition = timePosition;
    frame->getInfo().utcTime = utcTime;
    return SNRet::Status::SUCCESS;
  }

  void SNFFDecoder::flushDecoder() {
    avcodec_flush_buffers(mDecoder->codecCtx);
  }

}// namespace Sivin