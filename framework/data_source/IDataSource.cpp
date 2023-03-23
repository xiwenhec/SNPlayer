//
// Created by sivin on 11/28/22.
//
#include "IDataSource.h"

#include <utility>

namespace Sivin {

    IDataSource::IDataSource(std::string url) : mUrl(std::move(url)) {}

    void IDataSource::setConfig(SourceConfig &config) {
        mConfig = config;
    }

    void IDataSource::setInterrupt(bool interrupt) {
        mInterrupt = interrupt;
    }

    void IDataSource::setRange(int64_t start, int64_t end) {
        mRangeStart = start;
        mRangeEnd = end;
    }

}