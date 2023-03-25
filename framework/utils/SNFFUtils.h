//
// Created by sivin on 12/7/22.
//

#ifndef DATASOURCETEST_SNFFUTILS_H
#define DATASOURCETEST_SNFFUTILS_H

#include <base/media/SNMediaInfo.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include "libavutil/avutil.h"
}

namespace Sivin {

  class SNFFUtils {
  public:
    static const char *getErrorString(int err);

    static SNCodecID AVCodecToSNCodec(enum AVCodecID codecID);

    static SNPixelFormat AVPixelFormatToSNPixeFormat(int format);

    static SNSampleFormat AVSampleFormatToSNSampleFormat(int format);

    static int getStreamInfo(const struct AVStream *stream, SNStreamInfo *info);
  };

}// namespace Sivin

#endif//DATASOURCETEST_SNFFUTILS_H
