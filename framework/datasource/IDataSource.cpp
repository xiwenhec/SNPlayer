//
// Created by sivin on 11/28/22.
//
#include "IDataSource.h"

#include <utility>

namespace Sivin {

    IDataSource::IDataSource(std::string url) : mUrl(std::move(url)) {}


}