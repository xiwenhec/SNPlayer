//
// Created by sivin on 12/6/22.
//

#include "DataSourceFactory.h"
#include "data_source/curl/CurlDataSource.h"

namespace Sivin {
  IDataSource *DataSourceFactory::create(const std::string &url) {
    return new CurlDataSource(url);
  }
}// namespace Sivin