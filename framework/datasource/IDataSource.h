//
// Created by Sivin on 2022-11-26.
//

#ifndef SIVINPLAYER_IDATASOURCE_H
#define SIVINPLAYER_IDATASOURCE_H

#include <string>
#include <vector>
#include "utils/SNJson.h"

namespace Sivin {

    class IDataSource {
    public:
        class Listener {
        public:
            enum NetWorkRetryStatus {
                netWorkRetryStatus_pending,
                netWorkRetryStatus_retry,
            };
            enum BitStreamType {
                bitStreamType_media,
            };

            enum NetworkEvent {
                networkEvent_startConnect,
                networkEvent_connected,
                networkEvent_disconnect,
                networkEvent_error,
                networkEvent_eos,
                networkEvent_retry,
                networkEvent_resume,
                networkEvent_exit,
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
            enum IpResolveType {
                IpResolveWhatEver, IpResolveV4, IpResolveV6
            };

        public:
            /**
             * 最低传输速度
             */
            int lowSpeedLimit{1};
            int lowSpeedTime_ms{15000};
            /**
             * 连接超时时间
             */
            int connectTimeout_ms{15000};
            int so_rcv_size{0};

            std::string httpProxy;
            std::string refer; //请求源标识
            std::string userAgent;
            std::vector<std::string> customHeaders;
            std::shared_ptr<Listener> listener{nullptr};
            IpResolveType resolveType{IpResolveWhatEver};
            bool enableLog{true};

            std::string toString();
        };


    };

} // Sivin

#endif //SIVINPLAYER_IDATASOURCE_H
