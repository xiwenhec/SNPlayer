//
// Created by sivin on 12/25/22.
//

#ifndef DATASOURCETEST_SNAVPACKET_H
#define DATASOURCETEST_SNAVPACKET_H

extern "C" {
#include <libavcodec/packet.h>
};

#include "base/media/ISNPacket.h"
#include <string>


namespace Sivin {

    class SNAVPacket : public ISNPacket {

    public:
        explicit SNAVPacket(AVPacket **pkt);

        ~SNAVPacket();

        uint8_t *getData() override;

        int64_t getSize() override;

    private:
        AVPacket *mpkt{nullptr};
        bool mIsProtected;
        std::string mMagicKey{};

        void copyInfo();
    };

} // Sivin

#endif //DATASOURCETEST_SNAVPACKET_H
