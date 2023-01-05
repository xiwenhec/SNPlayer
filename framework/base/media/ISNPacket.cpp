//
// Created by sivin on 12/6/22.
//

#include "ISNPacket.h"

namespace Sivin {


    ISNPacket::PacketInfo &ISNPacket::PacketInfo::operator=(const ISNPacket::PacketInfo &other) {
        if (this == &other) {
            return *this;
        }
        this->streamIndex = other.streamIndex;
        this->pts = other.pts;
        this->dts = other.dts;
        this->flags = other.flags;
        return *this;
    }
} // Sivin