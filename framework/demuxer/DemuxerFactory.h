//
// Created by sivin on 1/1/23.
//

#ifndef SN_FRAMEWORK_DEMUXERFACTORY_H
#define SN_FRAMEWORK_DEMUXERFACTORY_H
#include "demuxer/IDemuxer.h"
namespace Sivin {

  class DemuxerFactory {
  public:
    static IDemuxer *createDemuxer(const std::string &path);
  };

}// namespace Sivin

#endif//SN_FRAMEWORK_DEMUXERFACTORY_H
