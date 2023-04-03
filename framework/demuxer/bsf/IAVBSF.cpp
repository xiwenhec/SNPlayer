//
// Created by sivin on 12/7/22.
//

#include <utils/SNLog.h>
#include "IAVBSF.h"
#include "utils/SNFFUtil.h"

namespace Sivin {

  AVBSF::~AVBSF() {
    if (mBsfContext != nullptr) {
      av_bsf_free(&mBsfContext);
    }
  }

  int AVBSF::init(const std::string &name, AVCodecParameters *codecpar) {
    const AVBitStreamFilter *bsf = av_bsf_get_by_name(name.c_str());
    if (!bsf) {
      SN_LOGE("%s bsf not found", name.c_str());
      return -1;
    }
    int ret = av_bsf_alloc(bsf, &mBsfContext);
    if (ret < 0) {
      SN_LOGE("Cannot alloc bsf");
      return ret;
    }

    avcodec_parameters_copy(mBsfContext->par_in, codecpar);
    ret = av_bsf_init(mBsfContext);
    if (ret < 0) {
      SN_LOGE("Error initializing bitstream filter: %s", bsf->name);
      return ret;
    }
    ret = avcodec_parameters_copy(codecpar, mBsfContext->par_out);
    if (ret < 0) {
      return ret;
    }
    return 0;
  }

  int AVBSF::sendPacket(AVPacket *pkt) {
    int ret = av_bsf_send_packet(mBsfContext, pkt);
    if (ret < 0) {
      SN_LOGE("av_bsf_send_packet error %d (%s)", ret, SNFFUtil::getErrorString(ret));
    }
    return ret;
  }

  int AVBSF::receivePacket(AVPacket *pkt) {
    int ret = av_bsf_receive_packet(mBsfContext, pkt);
    if (ret == AVERROR_EOF) {
      return 0;
    } else if (ret < 0) {
      SN_LOGE("av_bsf_receive_packet error %d (%s)", ret, SNFFUtil::getErrorString(ret));
      return ret;
    } else {
      return pkt->size;
    }
  }



}// namespace Sivin