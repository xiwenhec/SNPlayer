//
// Created by sivin on 12/6/22.
//

#ifndef SN_FRAMEWORK_SNUTILS_H
#define SN_FRAMEWORK_SNUTILS_H

#include <string>

namespace Sivin {
  class SNUtils {
  public:
    static bool startWith(const std::string &src, const std::initializer_list<std::string> &val);
  };
}// namespace Sivin

#endif//SN_FRAMEWORK_SNUTILS_H
