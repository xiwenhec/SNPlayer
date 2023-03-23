//
// Created by sivin on 12/6/22.
//

#ifndef DATASOURCETEST_DATASOURCEFACTORY_H
#define DATASOURCETEST_DATASOURCEFACTORY_H

#include <memory>
#include "data_source/IDataSource.h"

namespace Sivin {
  class DataSourceFactory {
  public:
    static std::shared_ptr<IDataSource> create(const std::string &url);
  };
}


#endif //DATASOURCETEST_DATASOURCEFACTORY_H
