#include "DecoderFactory.h"

namespace Sivin {
  std::unique_ptr<IDecoder> DecoderFactory::create() {
    return std::unique_ptr<IDecoder>(new SNFFDecoder());
  }
}// namespace Sivin