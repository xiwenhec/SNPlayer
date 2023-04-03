#pragma once
#include "demuxer/bsf/IAVBSF.h"
#include <string>
#include <memory>
namespace Sivin {
  class AVBSFFactory {
  public:
    static std::unique_ptr<IAVBSF> create(const std::string &name);
  };
}// namespace Sivin