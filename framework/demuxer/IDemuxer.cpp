//
// Created by sivin on 12/6/22.
//

#include "IDemuxer.h"

namespace Sivin {
    IDemuxer::IDemuxer(std::string path) : mPath(std::move(path)) {
    }

    void IDemuxer::setDataCallback(ReadCallback readCallback, SeekCallback seekCallback, void *userArgs) {
        mReadCb = readCallback;
        mSeekCb = seekCallback;
        mUserArg = userArgs;
    }


} // Sivin