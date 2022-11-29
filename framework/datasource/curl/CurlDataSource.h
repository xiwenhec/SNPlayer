//
// Created by sivin on 11/28/22.
//

#ifndef DATASOURCETEST_CURLDATASOURCE_H
#define DATASOURCETEST_CURLDATASOURCE_H

#include "datasource/IDataSource.h"

namespace Sivin {
    class CurlDataSource : IDataSource {

        explicit CurlDataSource(const std::string &url);
        ~CurlDataSource();

        int open(int flags) override;

        int read(void *outBuffer, size_t size) override;

        void close() override;

        int64_t seek(int64_t offset, int whence) override;


    private:
        std::string mUri;
    };
}


#endif //DATASOURCETEST_CURLDATASOURCE_H
