#ifndef SN_FRAMEWORK_DECODERFACTORY_H
#define SN_FRAMEWORK_DECODERFACTORY_H

#include "base/media/SNMediaInfo.h"
#include "codec/IDecoder.h"
#include "codec/SNFFDecoder.h"
#include <memory>
#include <mutex>
namespace Sivin {

  class DecoderFactory {
  public:
    static std::unique_ptr<IDecoder> create();
  };

}// namespace Sivin


#endif//