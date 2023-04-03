//
// Created by sivin on 23-1-15.
//

#include "PlayerParams.h"

#define DEFAULTSTART_BUFFER_DURATION (1000 * 1000)
#define DEFAULT_MAX_BUFFER_DURATION (10 * 1000 * 1000)

namespace Sivin {

  PlayerParams::PlayerParams() {
    reset();
  }

  PlayerParams::~PlayerParams() {
  }

  void PlayerParams::reset() {
    timeoutMs = 15000;
    startBufferDuration = DEFAULTSTART_BUFFER_DURATION;
    maxBufferDuration = DEFAULT_MAX_BUFFER_DURATION;
    lowMemSize = 100 * 1024 * 1024;//100M
  }

}// namespace Sivin