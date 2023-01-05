//
// Created by sivin on 1/2/23.
//

#include "ICodec.h"
#include "utils/NSLog.h"
extern "C" {
#include "libavformat/avformat.h"
}


namespace Sivin {
    void ICodec::test() {
        char sourcePath[] = "./res/ceshi.mp4";
        AVFormatContext *mCtx = NULL;
        int ret = avformat_open_input(&mCtx, sourcePath, NULL, NULL);
        SN_LOGE("ret = %d",ret);
    }
}

