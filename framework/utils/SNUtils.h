//
// Created by sivin on 12/6/22.
//

#ifndef DATASOURCETEST_SNUTILS_H
#define DATASOURCETEST_SNUTILS_H

#include <string>

namespace Sivin {
    class SNUtils {
    public:
        static bool startWith(const std::string &src, const std::initializer_list<std::string> &val);
    };
}

#endif //DATASOURCETEST_SNUTILS_H
