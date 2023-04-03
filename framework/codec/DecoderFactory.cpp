#include "DecoderFactory.h"
#include "codec/SNFFDecoder.h"
namespace Sivin {
  std::unique_ptr<IDecoder> DecoderFactory::create() {
    return std::unique_ptr<IDecoder>(new SNFFDecoder());
  }
}// namespace Sivin