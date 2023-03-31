#ifndef SN_FRAMKEWORK_SNMEDIAINFO_H
#define SN_FRAMKEWORK_SNMEDIAINFO_H
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
    SN_CODEC_ID_NONE = -1,
    SN_CODEC_ID_H264,
    SN_CODEC_ID_H265,
    SN_CODEC_ID_AAC,
  };

  enum class SNSampleFormat {
    FMT_NONE = -1,
    FMT_S16,
  };

  enum class SNPixelFormat {
    FMT_NONE = -1,
    FMT_YUV420P,
  };


  struct SNStreamInfo {

    StreamType type;
    int64_t duration;
    SNCodecID codeId;
    uint32_t codec_tag;
    int index;
    int64_t bitrate;
    int profile;//例如H264中的Baseline，Main，High等,音频AAC编码中AAC-LC

    int channels;
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

  struct SNMeidaInfo {
    int64_t totalBitrate;
  };

};// namespace Sivin

#endif//SN_FRAMKEWORK_BASE_MEDIA_SNMEDIAINFO_H