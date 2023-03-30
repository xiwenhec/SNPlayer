#ifndef SN_FRAMEWORK_IVIDEORENDER_H
#define SN_FRAMEWORK_IVIDEORENDER_H

#include <cstdint>
#include <memory>
#include "base/media/SNFrame.h"

namespace Sivin {
  class IVideoRender {
  public:
    enum class Rotate {
      Rotate_None = 0,
      Rotate_90 = 90,
      Rotate_180 = 180,
      Rotate_270 = 270
    };

    enum class Flip {
      Flip_None,
      Flip_Horizontal,
      Flip_Vertical,
      Flip_Both,
    };

    enum class Scale {
      Scale_AspectFit,
      Scale_AspectFill,
      Scale_Fill
    };

  public:
    explicit IVideoRender() = default;
    virtual ~IVideoRender() = default;

  public:
    virtual int init() = 0;

    virtual int clearScreen() = 0;

    virtual void setBackgroundColor(uint32_t color) = 0;

    virtual int renderFrame(std::unique_ptr<SNFrame> &frame) = 0;

    virtual int setRotate(Rotate rotate) = 0;

    virtual int setFlip(Flip flip) = 0;

    virtual int setScale(Scale scale) = 0;

    virtual void setSpeed(float speed) = 0;

    virtual void setWindowSize(int windWith, int windHeight) = 0;

    virtual int setDisPlay(void *view);
  };

}// namespace Sivin


#endif