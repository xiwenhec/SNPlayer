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
        enum DemuxerType {
            DEMUXER_TYPE_UNKNOWN = 0,
            DEMUXER_TYPE_BITSTREAM,
        };

        enum BitStreamType {
            BITSTREAM_TYPE_NO_TOUCH,
            BITSTREAM_TYPE_MERGE,
            BITSTREAM_TYPE_EXTRACT,
        };

    public:
        using ReadCallback = int (*)(std::shared_ptr<void> userArg, uint8_t *buffer, int size);
        using SeekCallback = int64_t(*)(std::shared_ptr<void> arg, int64_t offset, int whence);
    public:
        explicit IDemuxer(std::string path);

        void setDataCallback(ReadCallback readCallback);

        void setBitStreamType(BitStreamType videoStreamType, BitStreamType audioStreamType) {
            mVideoStreamType = videoStreamType;
            mAudioStreamType = audioStreamType;
        }

    protected:
        std::string mPath;
        ReadCallback mReadCb{nullptr};
        SeekCallback mSeekCb{nullptr};
        std::shared_ptr<void> mUserArg{nullptr};
        BitStreamType mVideoStreamType = BITSTREAM_TYPE_NO_TOUCH;
        BitStreamType mAudioStreamType = BITSTREAM_TYPE_NO_TOUCH;
    };

} // Sivin

#endif //DATASOURCETEST_IDEMUXER_H
