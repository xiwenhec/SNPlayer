//
// Created by sivin on 1/1/23.
//

#include "DemuxerPrototype.h"
#include "demuxer/AVFormatDemuxer.h"
namespace Sivin {
    std::unique_ptr<IDemuxer> DemuxerPrototype::createDemuxer(std::string path) {
        return std::make_unique<AVFormatDemuxer>(std::move(path));
    }
} // Sivin