//
// Created by sivin on 11/28/22.
//

#ifndef DATASOURCETEST_CURLDATASOURCE_H
#define DATASOURCETEST_CURLDATASOURCE_H

#include "data_source/IDataSource.h"
#include "data_source/curl/CurlConnectionManager.h"
#include "data_source/curl/CurlConnection.h"
#include <shared_mutex>

namespace Sivin {
  class CurlDataSource : public IDataSource {
  public:
    explicit CurlDataSource(const std::string &url);

    ~CurlDataSource();

    int open(int flags) override;

    int64_t read(void *outBuffer, int64_t size) override;

    void close() override;

    int64_t seek(int64_t offset, int whence) override;

    std::string getUri() override;

  private:
    std::shared_ptr<CurlConnection> initConnection();

    void closeConnection(bool forbidReuse);

    int64_t trySeekByNewConnection(int64_t offset);

  private:
    std::string mUri;
    std::shared_ptr<CurlConnection> mConnection;
    std::shared_ptr<CurlConnectionManager> mConnectionManager;
    std::atomic<bool> mNeedReconnect{false};
    curl_slist *mHeaderList{nullptr};
    int64_t mFileSize{-1};
    std::shared_mutex mSharedMutex;
  };
}


#endif //DATASOURCETEST_CURLDATASOURCE_H
