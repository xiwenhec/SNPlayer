//
// Created by sivin on 12/6/22.
//

#ifndef DATASOURCETEST_IDEMUXER_H
#define DATASOURCETEST_IDEMUXER_H

#include <string>


namespace Sivin {

    class IDemuxer {

    public:
        using ReadCallback = int (*)(void *arg, uint8_t *buffer, int size);

    public:
        explicit IDemuxer(std::string path);

        void setDataCallback(ReadCallback readCallback);

    private:
        std::string mPath;
        ReadCallback mReadCallback{nullptr};
    };

} // Sivin

#endif //DATASOURCETEST_IDEMUXER_H
