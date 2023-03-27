//
// Created by sivin on 1/2/23.
//

#ifndef DATASOURCETEST_ICODEC_H
#define DATASOURCETEST_ICODEC_H

#include <cstdint>
namespace Sivin {
  class IDecoder {
    explicit IDecoder() = default;
    virtual ~IDecoder() = default;

  public:
    virtual uint32_t getInputQueueSize() = 0;



  };
}// namespace Sivin


#endif//DATASOURCETEST_ICODEC_H
