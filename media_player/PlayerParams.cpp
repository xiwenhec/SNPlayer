//
// Created by sivin on 23-1-15.
//

#include "PlayerParams.h"

namespace Sivin {

  PlayerParams::PlayerParams() {
  }

  PlayerParams::~PlayerParams() {
  }

  void PlayerParams::reset() {
    timeoutMs = 15000;
  }

}// namespace Sivin