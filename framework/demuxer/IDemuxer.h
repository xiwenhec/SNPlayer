//
// Created by sivin on 12/6/22.
//

#ifndef DATASOURCETEST_IDEMUXER_H
#define DATASOURCETEST_IDEMUXER_H

#include <string>
#include <memory>

namespace Sivin {

    class IDemuxer {

    public:
        using ReadCallback = int (*)(std::shared_ptr<void> userArg, uint8_t *buffer, int size);
        using SeekCallback = int64_t(*)(std::shared_ptr<void> arg, int64_t offset, int whence);
    public:
        explicit IDemuxer(std::string path);

        void setDataCallback(ReadCallback readCallback);

    protected:
        std::string mPath;
        ReadCallback mReadCb{nullptr};
        SeekCallback mSeekCb{nullptr};
        std::shared_ptr<void> mUserArg{nullptr};
    };

} // Sivin

#endif //DATASOURCETEST_IDEMUXER_H
