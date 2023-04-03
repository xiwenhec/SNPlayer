#include "SNAVFrame.h"
#include "base/media/SNFrame.h"
#include "utils/SNFFUtil.h"
#include <cassert>
#include <memory>
namespace Sivin {

  SNAVFrame::SNAVFrame(AVFrame *frame) : mAVFrame(av_frame_clone(frame)) {
    assert(mAVFrame != nullptr);
    copyInfo();
  }
  SNAVFrame::~SNAVFrame() {
    if (mAVFrame) {
      av_frame_free(&mAVFrame);
      mAVFrame = nullptr;
    }
  }

  SNFrame::FrameType SNAVFrame::getType() {
    if (mType != FrameType::UnKnown) {
      return mType;
    }

    if (mAVFrame->width > 0 && mAVFrame->height > 0) {
      return FrameType::Video;
    }

    if (mAVFrame->nb_samples > 0 && mAVFrame->ch_layout.nb_channels > 0) {
      return FrameType::Audio;
    }

    return FrameType::UnKnown;
  }

  void SNAVFrame::copyInfo() {
    if (mType == FrameType::UnKnown) {
      mType = getType();
    }
    mInfo.pts = mAVFrame->pts;
    mInfo.pkt_dts = mAVFrame->pkt_dts;
    mInfo.duration = mAVFrame->duration;
    if (mType == FrameType::Audio) {
      mInfo.audio.channels = mAVFrame->ch_layout.nb_channels;
      mInfo.audio.nb_samples = mAVFrame->nb_samples;
      mInfo.audio.sample_rate = mAVFrame->sample_rate;
      mInfo.audio.format = SNFFUtil::AVSampleFormatToSNSampleFormat(mAVFrame->format);
    } else if (mType == FrameType::Video) {
      mInfo.key_frame = mAVFrame->key_frame;
      mInfo.video.height = mAVFrame->height;
      mInfo.video.width = mAVFrame->width;
      mInfo.video.format = SNFFUtil::AVPixelFormatToSNPixeFormat(mAVFrame->format);
    }
  }

  uint8_t **SNAVFrame::getData() {
    return mAVFrame->data;
  }

  int *SNAVFrame::getLineSize() {
    return mAVFrame->linesize;
  }

  std::unique_ptr<SNFrame> SNAVFrame::clone() {
    return std::unique_ptr<SNFrame>();
  }

}// namespace Sivin