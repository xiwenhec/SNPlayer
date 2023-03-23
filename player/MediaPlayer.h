//
// Created by sivin on 23-1-15.
//

#ifndef SIVINPLAYER_MEDIAPLAYER_H
#define SIVINPLAYER_MEDIAPLAYER_H

#include "string"
#include "MediaPlayerDef.h"

namespace Sivin {

    class MediaPlayer {
    public:
        explicit MediaPlayer();

        ~MediaPlayer();

        static std::string GetSdkVersion() {
            return "Sivin:0.1";
        }

    public:

        //设置显示的view
        void setView(void *view);

        //设置播放URL
        void setDataSource(const std::string &url);

        void prepare();

        void start();

        void pause();

        void seekTo(int64_t seekPos, SeekMode mode);

        void Stop();

    };

} // Sivin

#endif //SIVINPLAYER_MEDIAPLAYER_H
