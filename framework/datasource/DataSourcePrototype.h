//
// Created by sivin on 12/6/22.
//

#ifndef DATASOURCETEST_DATASOURCEPROTOTYPE_H
#define DATASOURCETEST_DATASOURCEPROTOTYPE_H

#include <memory>
#include "datasource/IDataSource.h"

namespace Sivin {
    class DataSourcePrototype {
    public:
        static std::shared_ptr<IDataSource> create(const std::string &url);
    };
}


#endif //DATASOURCETEST_DATASOURCEPROTOTYPE_H
