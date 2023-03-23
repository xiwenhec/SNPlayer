//
// Created by sivin on 23-1-15.
//

#ifndef SIVINPLAYER_PLAYERDEFINITION_H
#define SIVINPLAYER_PLAYERDEFINITION_H

namespace Sivin {

  enum class SeekMode {
    ACCURATE = 0x01,
    INACCURATE = 0x10,
  };

  enum class PlayerStatus {
    IDLE = 0,
    INITIALIZED,
    PREPARING,
    PREPARED,
    PLAYING,
    PAUSED,
    STOPPED,
    COMPLETION,
    ERROR = 101,
  };


}

#endif //SIVINPLAYER_PLAYERDEFINITION_H
