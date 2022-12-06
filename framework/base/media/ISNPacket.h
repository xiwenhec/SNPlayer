//
// Created by sivin on 12/6/22.
//

#ifndef DATASOURCETEST_ISNPACKET_H
#define DATASOURCETEST_ISNPACKET_H

#include <cstdint>


namespace Sivin {

    class ISNPacket {
    public:
        class PacketInfo {
            int streamIndex;
            int64_t pts;
            int64_t dts;
            int flags;
        public:
            PacketInfo &operator=(const PacketInfo &other);
        };



    };

} // Sivin

#endif //DATASOURCETEST_ISNPACKET_H
