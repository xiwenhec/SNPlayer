//
// Created by sivin on 1/1/23.
//

#ifndef DATASOURCETEST_DEMUXERPROTOTYPE_H
#define DATASOURCETEST_DEMUXERPROTOTYPE_H
#include "demuxer/IDemuxer.h"
namespace Sivin {

    class DemuxerPrototype {
    public:
        static std::unique_ptr<IDemuxer> createDemuxer(std::string path);
    };

} // Sivin

#endif //DATASOURCETEST_DEMUXERPROTOTYPE_H
