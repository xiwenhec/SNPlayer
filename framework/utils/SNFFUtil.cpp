//
// Created by sivin on 12/7/22.
//

#include "SNFFUtil.h"
#include "base/media/SNMediaInfo.h"

namespace Sivin {
  static char errorBuff[256];
  const char *SNFFUtil::getErrorString(int err) {
    av_strerror(err, errorBuff, sizeof(errorBuff));
    return errorBuff;
  }

  SNCodecID SNFFUtil::AVCodecToSNCodec(enum AVCodecID codecID) {
    switch (codecID) {
      case AV_CODEC_ID_AAC:
      case AV_CODEC_ID_AAC_LATM:
        return SNCodecID::SN_CODEC_ID_AAC;
      case AV_CODEC_ID_H264:
        return SNCodecID::SN_CODEC_ID_H264;
      case AV_CODEC_ID_HEVC:
        return SNCodecID::SN_CODEC_ID_H265;
      default:
        return SNCodecID::SN_CODEC_ID_NONE;
    }
    return SNCodecID::SN_CODEC_ID_NONE;
  }

  enum AVCodecID SNFFUtil::SNCodecToAVCodec(SNCodecID codecId) {
    switch (codecId) {
      case SNCodecID::SN_CODEC_ID_H264:
        return AV_CODEC_ID_H264;
      case SNCodecID::SN_CODEC_ID_H265:
        return AV_CODEC_ID_HEVC;
      case SNCodecID::SN_CODEC_ID_AAC:
        return AV_CODEC_ID_AAC;
      default:
        return AV_CODEC_ID_NONE;
    }
  }


  SNPixelFormat SNFFUtil::AVPixelFormatToSNPixeFormat(int format) {
    switch (format) {
      case AV_PIX_FMT_YUV420P:
        return SNPixelFormat::SN_PIX_FMT_YUV420P;
      default:
        return SNPixelFormat::SN_PIX_FMT_NONE;
    }
  }

  SNSampleFormat SNFFUtil::AVSampleFormatToSNSampleFormat(int format) {
    switch (format) {
      case AV_SAMPLE_FMT_S16:
        return SNSampleFormat::SN_SAMPLE_FMT_S16;
      default:
        return SNSampleFormat::SN_SAMPLE_FMT_NONE;
    }
  }

  int SNFFUtil::getStreamInfo(const struct AVStream *stream, SNStreamInfo *info) {
    enum AVMediaType codeType = stream->codecpar->codec_type;
    memset(info, 0, sizeof(SNStreamInfo));
    info->type = StreamType::STREAM_TYPE_UNKNOWN;
    info->codeId = SNFFUtil::AVCodecToSNCodec(stream->codecpar->codec_id);
    info->codec_tag = stream->codecpar->codec_tag;
    info->ptsTimeBase = (float) stream->time_base.num * 1000000 / (float) stream->time_base.den;
    info->pixFormat = SNPixelFormat::SN_PIX_FMT_NONE;
    info->sampleFormat = SNSampleFormat::SN_SAMPLE_FMT_NONE;

    if (codeType == AVMEDIA_TYPE_VIDEO) {
      //stream里的表示解码后的视频宽高比，codecpar的表示原始图像宽高比
      if (stream->sample_aspect_ratio.num && av_cmp_q(stream->sample_aspect_ratio, stream->codecpar->sample_aspect_ratio)) {
        info->displayWidth = stream->codecpar->width;
        info->displayWidth *= stream->sample_aspect_ratio.num;
        info->displayWidth /= stream->sample_aspect_ratio.den;
        info->displayHeight = stream->codecpar->height;
      } else if (stream->codecpar->sample_aspect_ratio.num) {
        info->displayWidth = stream->codecpar->width;
        info->displayWidth *= stream->codecpar->sample_aspect_ratio.num;
        info->displayWidth /= stream->codecpar->sample_aspect_ratio.den;
        info->displayHeight = stream->codecpar->height;
      } else {
        info->displayWidth = info->displayHeight = 0;
      }

      info->type = StreamType::STREAM_TYPE_VIDEO;
      info->width = stream->codecpar->width;
      info->height = stream->codecpar->height;
      info->profile = stream->codecpar->profile;
      info->pixFormat = SNFFUtil::AVPixelFormatToSNPixeFormat(stream->codecpar->format);

      if (stream->avg_frame_rate.num && stream->avg_frame_rate.den) {
        info->avgFps = av_q2d(stream->avg_frame_rate);
      } else if (stream->r_frame_rate.den && stream->r_frame_rate.num) {
        info->avgFps = av_q2d(stream->r_frame_rate);
      } else {
        info->avgFps = 0;
      }
    } else if (codeType == AVMEDIA_TYPE_AUDIO) {
      info->type = StreamType::STREAM_TYPE_AUDIO;
      info->channels = stream->codecpar->ch_layout.nb_channels;
      info->sampleRate = stream->codecpar->sample_rate;
      info->sampleFormat = SNFFUtil::AVSampleFormatToSNSampleFormat(stream->codecpar->format);
      info->profile = stream->codecpar->profile;
    }

    info->extraDataSize = stream->codecpar->extradata_size;
    info->extraData = static_cast<uint8_t *>(malloc(stream->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE));
    memcpy(info->extraData, stream->codecpar->extradata, stream->codecpar->extradata_size);
    return 0;
  }

}// namespace Sivin