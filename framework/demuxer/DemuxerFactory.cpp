//
// Created by sivin on 1/1/23.
//

#include "DemuxerFactory.h"
#include "demuxer/AVFormatDemuxer.h"
#include <utility>
namespace Sivin {
  IDemuxer *DemuxerFactory::createDemuxer(const std::string &path) {
    return new AVFormatDemuxer(path);
  }
}// namespace Sivin