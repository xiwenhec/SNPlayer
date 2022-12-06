//
// Created by sivin on 12/6/22.
//

#include "SNUtils.h"

namespace Sivin {
    bool SNUtils::startWith(const std::string &src, const std::initializer_list<std::string> &val) {
        return std::any_of(val.begin(), val.end(), [&src](const std::string &item) {
            return src.substr(0, item.length()) == item;
        });
    }
}