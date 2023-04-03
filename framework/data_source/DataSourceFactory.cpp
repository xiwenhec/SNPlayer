//
// Created by sivin on 12/6/22.
//

#include "DataSourceFactory.h"
#include "data_source/IDataSource.h"
#include "data_source/curl/CurlDataSource.h"
#include <memory>

namespace Sivin {
  std::unique_ptr<IDataSource> DataSourceFactory::create(const std::string &url) {
    return std::make_unique<CurlDataSource>(url);
  }
}// namespace Sivin