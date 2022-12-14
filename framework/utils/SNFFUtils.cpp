//
// Created by sivin on 12/7/22.
//

#include "SNFFUtils.h"

extern "C" {
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#include "libavutil/avutil.h"
}

namespace Sivin {
    static char errorBuff[256];
    const char *SNFFUtils::getErrorString(int err) {
       av_strerror(err, errorBuff, sizeof(errorBuff));
       return errorBuff;
    }
} // Sivin