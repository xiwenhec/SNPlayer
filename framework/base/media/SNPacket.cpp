//
// Created by sivin on 12/6/22.
//

#include "SNPacket.h"

namespace Sivin {


    SNPacket::PacketInfo &SNPacket::PacketInfo::operator=(const SNPacket::PacketInfo &other) {
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