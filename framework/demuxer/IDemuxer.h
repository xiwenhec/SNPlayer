//
// Created by sivin on 12/6/22.
//

#ifndef SN_FRAMEWORK_IDEMUXER_H
#define SN_FRAMEWORK_IDEMUXER_H

#include <string>
#include <memory>
#include <functional>
#include "base/media/SNPacket.h"
#include "base/SNRet.h"
#include <base/media/SNMediaInfo.h>

namespace Sivin {

  //表示解封装后期待的的目标码流类型
  enum class BitStreamType {
    //和封装类型保持一致，不做任何改变
    NO_TOUCH,

    //修改成类似于Annex b的格式
    MERGE,

    //修改成AVCC的格式
    EXTRACT,
  };

  class IDemuxer {

  public:
    enum DemuxerType {
      DEMUXER_TYPE_UNKNOWN = 0,
      DEMUXER_TYPE_BITSTREAM,
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

    virtual SNRet readPacket(std::unique_ptr<SNPacket> &packet, int index) = 0;

    virtual int openStream(int index) = 0;

    virtual void closeStream(int index) = 0;

  public:
    virtual int getMediaInfo(std::unique_ptr<SNMeidaInfo> &mediaInfo) = 0;

    virtual int getNbStreams() const = 0;

    virtual int getStreamInfo(std::unique_ptr<SNStreamInfo> &streamInfo, int index) = 0;

  protected:
    std::string mPath;
    ReadCallback mReadCb{nullptr};
    SeekCallback mSeekCb{nullptr};
    void *mUserArg{nullptr};
    BitStreamType mVideoStreamType{BitStreamType::NO_TOUCH};
    BitStreamType mAudioStreamType{BitStreamType::NO_TOUCH};
  };

}// namespace Sivin

#endif//SN_FRAMEWORK_IDEMUXER_H
