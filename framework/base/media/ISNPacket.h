//
// Created by sivin on 12/6/22.
//

#ifndef DATASOURCETEST_ISNPACKET_H
#define DATASOURCETEST_ISNPACKET_H

#include <cstdint>


namespace Sivin {

#define SN_PKT_FLAG_KEY     0x0001 ///< The packet contains a keyframe
#define SN_PKT_FLAG_CORRUPT 0x0002 ///< The packet content is corrupted

    class ISNPacket {
    public:
        class PacketInfo {
        public:
            int streamIndex;
            int64_t pts;
            int64_t dts;
            int64_t timePosition;
            int64_t duration;
            int flags;
            int64_t pos;
        public:
            PacketInfo &operator=(const PacketInfo &other);
        };

    public:

        virtual ~ISNPacket() = 0;

        virtual uint8_t *getData() = 0;

        virtual PacketInfo &getInfo() {
            return mInfo;
        };

        virtual int64_t getSize() = 0;

    protected:
        PacketInfo mInfo{};
    };

} // Sivin

#endif //DATASOURCETEST_ISNPACKET_H
