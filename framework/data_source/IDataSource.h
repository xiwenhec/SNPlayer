//
// Created by Sivin on 2022-11-26.
//
#ifndef SN_FRAMKEWORK_IDATASOURCE_H
#define SN_FRAMKEWORK_IDATASOURCE_H

#include <string>
#include <vector>
#include <atomic>
#include "utils/SNJson.h"

namespace Sivin {

  enum {
    SEEK_SIZE = 0x10000,
  };

  class IDataSource {
  public:
    class Listener {
    public:
      enum NetWorkRetryStatus {
        NETWORK_RETRY_STATUS_PENDING,
        NETWORK_RETRY_STATUS_RETRY,
      };
      enum BitStreamType {
        BITSTREAM_TYPE_MEDIA,
      };

      enum NetworkEvent {
        NETWORK_EVENT_START_CONNECT,
        NETWORK_EVENT_CONNECTED,
        NETWORK_EVENT_DISCONNECT,
        NETWORK_EVENT_EOS,
        NETWORK_EVENT_ERROR,
        NETWORK_EVENT_RETRY,
        NETWORK_EVENT_RESUME,
        NETWORK_EVENT_EXIT,
      };

      virtual void onNetworkEvent(const std::string &url, const SNJson &eventParams) {}

      virtual NetWorkRetryStatus onNetWorkRetry(int error) = 0;

      virtual void onNetWorkConnected() {};

      virtual bool onNetWorkInput(uint64_t size, BitStreamType type) {
        return false;
      }
    };

    class SourceConfig {
    public:
      enum IpType {
        IP_TYPE_ANY,
        IP_TYPE_V4,
        IP_TYPE_V6
      };

    public:
      /**
       * 最低传输速度
       */
      int lowSpeedLimit{1};
      int lowSpeedTimeMs{15000};

      /**
       * 连接超时时间
       */
      int connectTimeoutMs{15000};

      /**
       * socket 接收缓冲区大小
       */
      int sockReceiveBufferSize{0};

      std::string httpProxy{};
      std::string refer{}; //请求源标识
      std::string userAgent{};
      std::vector<std::string> customHeaders{};
      std::shared_ptr<Listener> listener{nullptr};
      IpType resolveType{IP_TYPE_ANY};
      bool enableLog{true};
    };

  public:
    explicit IDataSource(std::string url);

    virtual ~IDataSource() = default;

    virtual int open(int flags) = 0;

    virtual void close() = 0;

    virtual int64_t read(void *outBuffer, int64_t size) = 0;

    virtual int64_t seek(int64_t offset, int whence) = 0;

    virtual std::string getUri() = 0;

  public:
    void setConfig(SourceConfig &config);

    void setInterrupt(bool interrupt);

    void setRange(int64_t start, int64_t end);

  protected:
    std::string mUrl;
    SourceConfig mConfig{};
    std::atomic_bool mInterrupt{false};
    int64_t mRangeStart{-1};
    int64_t mRangeEnd{-1};
  };


} // Sivin

#endif //SN_FRAMKEWORK_IDATASOURCE_H
