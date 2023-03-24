#ifndef FRAMKEWORK_BASE_MEDIA_STREAMINFO_H
#define FRAMKEWORK_BASE_MEDIA_STREAMINFO_H
#include <cstdint>

namespace Sivin {

  enum class StreamType {
    STREAM_TYPE_UNKNOWN = -1,
    STREAM_TYPE_VIDEO,
    STREAM_TYPE_AUDIO,
    STREAM_TYPE_SUB,
    STREAM_TYPE_MIXED,
    STREAM_TYPE_NUM,
  };

  enum class SNCodecID {
    CODEC_ID_NONE,
    CODEC_ID_H264,
  };

  enum class SNSampleFormat {
    SN_SAMPLE_FMT_S16,
  };

  enum class SNPixelFormat {
    SN_PIX_FMT_NONE = -1,
    SN_PIX_FMT_YUV420P,
  };


  struct StreamInfo {
    
    StreamType type;
    int64_t duration;
    SNCodecID codeId;
    int index;
    int64_t bitrate;
    int profile;

    int channels;
    int channel_layout;
    int sampleRate;
    int frameSize;

    SNSampleFormat sampleFormat;

    //video only
    int width;
    int height;
    int rotate;
    int displayWidth;
    int displayHeight;
    int avgFps;
    SNPixelFormat pixFormat;

    uint8_t *extraData;
    int extraDataSize;

    float ptsTimeBase;
  };


};// namespace Sivin

#endif//FRAMKEWORK_BASE_MEDIA_STREAMINFO_H