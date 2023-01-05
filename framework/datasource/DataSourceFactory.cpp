//
// Created by sivin on 12/6/22.
//

#include "DataSourceFactory.h"
#include "datasource/curl/CurlDataSource.h"

namespace Sivin {
    std::shared_ptr<IDataSource> DataSourceFactory::create(const std::string &url) {
        std::shared_ptr<IDataSource> dataSource = std::make_shared<CurlDataSource>(url);
        return std::move(dataSource);
    }
}