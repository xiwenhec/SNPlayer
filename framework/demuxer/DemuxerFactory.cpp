//
// Created by sivin on 1/1/23.
//

#include "DemuxerFactory.h"
#include "demuxer/AVFFDemuxer.h"
#include "demuxer/IDemuxer.h"
#include <memory>
#include <utility>
namespace Sivin {
  std::unique_ptr<IDemuxer> DemuxerFactory::create(const std::string &path) {
    return std::make_unique<AVFFDemuxer>(path);
  }
}// namespace Sivin