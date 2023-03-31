#ifndef SN_FRAMKEWORK_SNFRAME_H
#define SN_FRAMKEWORK_SNFRAME_H

#include "base/media/SNMediaInfo.h"
#include <cstdint>
#include <memory>

namespace Sivin {

  /**
    这个类定义了解码用于渲染的视频或者音频媒体数据帧
  */
  class SNFrame {

  public:
    enum class FrameType {
      UnKnown,
      Video,
      Audio
    };

    struct VideoInfo {
      int width{0};
      int height{0};
      int rotate{0};
      SNPixelFormat format{SNPixelFormat::FMT_NONE};

      bool operator==(const VideoInfo &info) const {
        return this->width == info.width && this->height == info.height && this->format == info.format;
      }

      bool operator!=(const VideoInfo &info) const { return !operator==(info); }
    };

    struct AudioInfo {
      int nb_samples{0};
      int channels{0};
      int sample_rate{0};
      SNSampleFormat format{SNSampleFormat::FMT_NONE};
      bool operator==(const AudioInfo &info) const {
        return this->sample_rate == info.sample_rate && this->channels == info.channels &&
               this->format == info.format;
      }

      bool operator!=(const AudioInfo &info) const { return !operator==(info); }
    };


    struct SNFrameInfo {
      int64_t pts{INT64_MIN};
      int64_t pkt_dts{INT64_MIN};
      int64_t duration{0};
      int64_t timePosition{INT64_MIN};
      int64_t utcTime{INT64_MIN};
      bool key_frame{false};
      union {
        VideoInfo video;
        AudioInfo audio;
      };
    };

  public:
    SNFrame() = default;
    virtual ~SNFrame() = default;

  public:
    virtual std::unique_ptr<SNFrame> clone() = 0;

    virtual uint8_t **getData() = 0;

    virtual int *getLineSize() = 0;

    virtual FrameType getType() = 0;

    void setDiscard(bool discard) {
      mDiscard = discard;
    }

    bool isDiscard() {
      return mDiscard;
    }

    SNFrameInfo &getInfo() {
      return mInfo;
    }

    void setProtect(bool protect) {
      mProtected = protect;
    }

    bool isProtected() const {
      return mProtected;
    }

  protected:
    SNFrameInfo mInfo{};
    bool mDiscard{false};
    bool mProtected{false};
    FrameType mType{FrameType::UnKnown};
  };


};// namespace Sivin


#endif// SN_FRAMKEWORK_SNFRAME_H