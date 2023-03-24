//
// Created by sivin on 1/1/23.
//

#ifndef DATASOURCETEST_DEMUXERFACTORY_H
#define DATASOURCETEST_DEMUXERFACTORY_H
#include "demuxer/IDemuxer.h"
namespace Sivin {

  class DemuxerFactory {
  public:
    static IDemuxer *createDemuxer(const std::string &path);
  };

}// namespace Sivin

#endif//DATASOURCETEST_DEMUXERFACTORY_H
