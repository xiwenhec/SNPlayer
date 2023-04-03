#include "demuxer/bsf/AVBSFFactory.h"
#include <memory>

namespace Sivin {

  std::unique_ptr<IAVBSF> AVBSFFactory::create(const std::string &name) {
    return std::make_unique<AVBSF>();
  }


}// namespace Sivin