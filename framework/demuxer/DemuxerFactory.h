//
// Created by sivin on 1/1/23.
//

#ifndef DATASOURCETEST_DEMUXERFACTORY_H
#define DATASOURCETEST_DEMUXERFACTORY_H
#include "demuxer/IDemuxer.h"
namespace Sivin {

    class DemuxerFactory {
    public:
        static std::unique_ptr<IDemuxer> createDemuxer(std::string path);
    };

} // Sivin

#endif //DATASOURCETEST_DEMUXERFACTORY_H
