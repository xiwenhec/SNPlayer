//
// Created by sivin on 12/7/22.
//
#ifndef FRAMKEWORK_UTILS_SNFFUTIL_H
#define FRAMKEWORK_UTILS_SNFFUTIL_H

#include <base/media/SNMediaInfo.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include "libavutil/avutil.h"
}

namespace Sivin {

  class SNFFUtil {
  public:
    static const char *getErrorString(int err);

    static SNCodecID AVCodecToSNCodec(enum AVCodecID codecID);

    static SNPixelFormat AVPixelFormatToSNPixeFormat(int format);

    static SNSampleFormat AVSampleFormatToSNSampleFormat(int format);

    static int getStreamInfo(const struct AVStream *stream, SNStreamInfo *info);
  };

}// namespace Sivin

#endif//#ifndef FRAMKEWORK_UTILS_SNFFUTIL_H

