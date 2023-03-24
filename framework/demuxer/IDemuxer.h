//
// Created by sivin on 12/6/22.
//

#ifndef DATASOURCETEST_IDEMUXER_H
#define DATASOURCETEST_IDEMUXER_H

#include <string>
#include <memory>
#include <functional>
#include "base/media/ISNPacket.h"
#include <base/media/StreamInfo.h>

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
    using ReadCallback = int (*)(void *userArg, uint8_t *buffer, int size);
    using SeekCallback = int64_t (*)(void *userArg, int64_t offset, int whence);

  public:
    explicit IDemuxer(std::string path);

    virtual ~IDemuxer(){};

    void setDataCallback(ReadCallback readCallback, SeekCallback seekCallback, void *userArgs);

    void setBitStreamType(BitStreamType videoStreamType, BitStreamType audioStreamType) {
      mVideoStreamType = videoStreamType;
      mAudioStreamType = audioStreamType;
    }

    virtual int open() = 0;

    virtual void start() = 0;

    virtual void preStop() = 0;

    virtual void stop() = 0;

    virtual int readPacket(std::unique_ptr<ISNPacket> &packet, int index) = 0;

    virtual int openStream(int index) = 0;

    virtual void closeStream(int index) = 0;

    virtual void getStreamInfo(std::unique_ptr<StreamInfo> &streamInfo, int index) = 0;

  protected:
    std::string mPath;
    ReadCallback mReadCb{nullptr};
    SeekCallback mSeekCb{nullptr};
    void *mUserArg{nullptr};
    BitStreamType mVideoStreamType = BITSTREAM_TYPE_NO_TOUCH;
    BitStreamType mAudioStreamType = BITSTREAM_TYPE_NO_TOUCH;
  };

}// namespace Sivin

#endif//DATASOURCETEST_IDEMUXER_H
