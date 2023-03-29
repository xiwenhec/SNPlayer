//
// Created by sivin on 1/2/23.
//

#ifndef SN_FRAMEWORK_IDECODER_H
#define SN_FRAMEWORK_IDECODER_H

#include <cstdint>
namespace Sivin {


  class IDecoder {
  public:
    explicit IDecoder() = default;
    virtual ~IDecoder() = default;

  public:
    
    virtual uint32_t getInputQueueSize() = 0;
  };
}// namespace Sivin


#endif//SN_FRAMEWORK_IDECODER_H
