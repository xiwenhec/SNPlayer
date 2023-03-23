//
// Created by sivin on 12/3/22.
//

#ifndef DATASOURCETEST_CURLCONNECTIONMANAGER_H
#define DATASOURCETEST_CURLCONNECTIONMANAGER_H

#include "curl/curl.h"
#include "utils/SNThread.h"
#include <list>

namespace Sivin {
  class CurlConnection;

  class CurlConnectionManager {
  public:
    CurlConnectionManager();

    ~CurlConnectionManager();

  public:
    void addConnection(const std::shared_ptr<CurlConnection> &connection);

    void removeConnection(const std::shared_ptr<CurlConnection> &connection);

    void resumeConnection(const std::shared_ptr<CurlConnection> &connection);

  private:
    int loop();

    void applyPending();

  private:
    CURLM *mMultiHandle{nullptr};
    int mStillRunning{0};
    std::unique_ptr<SNThread> mLoopThread;
    std::mutex mMutex;
    std::list<std::shared_ptr<CurlConnection>> mAddList;
    std::list<std::shared_ptr<CurlConnection>> mRemoveList;
    std::list<std::shared_ptr<CurlConnection>> mResumeList;
  };

} // Sivin

#endif //DATASOURCETEST_CURLCONNECTIONMANAGER_H
