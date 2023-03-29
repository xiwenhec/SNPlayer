#include "SNFFDecoder.h"
#include "utils/SNFFUtil.h"
#include "utils/SNLog.h"
#include <cstddef>
#include <memory>

namespace Sivin {

  SNFFDecoder::SNFFDecoder() {
    name = "ff.decoder";
    mDecoder = std::make_unique<InternalDecoder>();
  }

  SNFFDecoder::~SNFFDecoder() {
  }

  int SNFFDecoder::initDecoder(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags) {
    enum AVCodecID codecId = SNFFUtil::SNCodecToAVCodec(streamInfo->codeId);
    mDecoder->codec = avcodec_find_decoder(codecId);

    if (mDecoder->codec == nullptr) {
      SN_LOGE("couldn't find codec:%d", codecId);
      return -1;
    }

    mDecoder->codecContext = avcodec_alloc_context3(mDecoder->codec);
    if (mDecoder->codecContext == nullptr) {
      SN_LOGE("codecContext alloc failed");
      return -1;
    }

    bool isAudio = streamInfo->channels > 0;

    if (isAudio) {
      mDecoder->codecContext->ch_layout.nb_channels = streamInfo->channels;
      mDecoder->codecContext->sample_rate = streamInfo->sampleRate;
    }

    if (streamInfo->extraData != nullptr && streamInfo->extraDataSize > 0) {
      mDecoder->codecContext->extradata = (uint8_t *) av_mallocz(streamInfo->extraDataSize + AV_INPUT_BUFFER_PADDING_SIZE);
      mDecoder->codecContext->extradata_size = streamInfo->extraDataSize;
    }
    mDecoder->codecContext->thread_count = 1;

    if (avcodec_open2(mDecoder->codecContext, mDecoder->codec, nullptr) < 0) {
      SN_LOGE("couldn't open codec");
      avcodec_free_context(&mDecoder->codecContext);
      return -1;
    }
    mDecoder->avFrame = av_frame_alloc();
    if (!isAudio) {
      mDecoder->videoInfo.width = mDecoder->codecContext->width;
      mDecoder->videoInfo.height = mDecoder->codecContext->height;
      mDecoder->videoInfo.pix_fmt = mDecoder->codecContext->pix_fmt;
    }
    return 0;
  }

}// namespace Sivin