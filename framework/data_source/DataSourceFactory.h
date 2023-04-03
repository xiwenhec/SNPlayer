//
// Created by sivin on 12/6/22.
//

#ifndef SN_FRAMEWORK_DATASOURCEFACTORY_H
#define SN_FRAMEWORK_DATASOURCEFACTORY_H

#include <memory>
#include "data_source/IDataSource.h"

namespace Sivin {
  class DataSourceFactory {
  public:
    static std::unique_ptr<IDataSource> create(const std::string &url);
  };
}


#endif //SN_FRAMEWORK_DATASOURCEFACTORY_H
