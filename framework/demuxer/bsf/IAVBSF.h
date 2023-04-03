//
// Created by sivin on 12/7/22.
//

#ifndef SN_FRAMKEWORK_IAVBSF_H
#define SN_FRAMKEWORK_IAVBSF_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/bsf.h>
}

#include <string>

namespace Sivin {

    class IAVBSF {
    public:
        virtual ~IAVBSF() = default;

        virtual int init(const std::string &name, AVCodecParameters *codecpar) = 0;

        virtual int sendPacket(AVPacket *pkt) = 0;

        virtual int receivePacket(AVPacket *pkt) = 0;
    };

    class AVBSF : public IAVBSF {
    public:
        AVBSF() = default;

        ~AVBSF() override;

        int init(const std::string &name, AVCodecParameters *codecpar) override;

        int sendPacket(AVPacket *pkt) override;

        int receivePacket(AVPacket *pkt) override;

    private:
        AVBSFContext *mBsfContext{nullptr};
    };

   
} // Sivin

#endif //SN_FRAMKEWORK_IAVBSF_H
