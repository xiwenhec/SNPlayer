#ifndef SN_FRAMKEWORK_SNFRAME_H
#define SN_FRAMKEWORK_SNFRAME_H

#include <cstdint>
#include <memory>

namespace Sivin {

  /**
    这个类定义了解码用于渲染的视频或者音频媒体数据帧
  */
  class SNFrame {

  public:
    enum class FrameType {
      FrameTypeUnKnown,
      FrameTypeVideo,
      FrameTypeAudio
    };

    struct VideoInfo {
      int width;
      int height;
      int rotate;
      int format;

      bool operator==(const VideoInfo &info) const {
        return this->width == info.width && this->height == info.height && this->format == info.format;
      }

      bool operator!=(const VideoInfo &info) const { return !operator==(info); }
    };

    struct AudioInfo {
      int nb_samples;
      int channels;
      int sample_rate;
      uint64_t channel_layout;//
      int format;
      bool operator==(const AudioInfo &info) const {
        return this->sample_rate == info.sample_rate && this->channels == info.channels &&
               this->format == info.format && this->channel_layout == info.channel_layout;
      }

      bool operator!=(const AudioInfo &info) const { return !operator==(info); }
    };


    struct SNFrameInfo {
      int64_t pts;
      int64_t pkt_dts;
      int64_t duration;
      int64_t timePosition;
      int64_t utcTime;
      bool key;
      union {
        VideoInfo video;
        AudioInfo audio;
      };
    };

  public:
    SNFrame() = default;
    virtual ~SNFrame() = default;

    virtual std::unique_ptr<SNFrame> clone() = 0;

    virtual uint8_t **getData() = 0;

    virtual int *getLineSize() = 0;

    virtual FrameType getType() = 0;

    virtual void setDiscard(bool discard) { mDiscard = discard; }

    virtual bool isDiscard() { return mDiscard; }

    SNFrameInfo &getInfo();

    void setProtect(bool protect) { mProtected = protect; }

    bool isProtected() const { return mProtected; }

  protected:
    SNFrameInfo mInfo{};
    bool mDiscard{false};
    bool mProtected{false};
  };


};// namespace Sivin


#endif// SN_FRAMKEWORK_SNFRAME_H