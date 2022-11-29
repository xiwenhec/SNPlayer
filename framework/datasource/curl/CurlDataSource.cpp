//
// Created by sivin on 11/28/22.
//

#include "CurlDataSource.h"

namespace Sivin {
    CurlDataSource::CurlDataSource(const std::string &url) : IDataSource(url) {}

    CurlDataSource::~CurlDataSource() {

    }


    int CurlDataSource::open(int flags) {


        return 0;
    }

    int CurlDataSource::read(void *outBuffer, size_t size) {


        return 0;
    }


    int64_t CurlDataSource::seek(int64_t offset, int whence) {


        return 0;
    }

    void CurlDataSource::close() {


    }


}