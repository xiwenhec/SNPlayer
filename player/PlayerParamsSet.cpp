//
// Created by sivin on 23-1-15.
//

#include "PlayerParamsSet.h"

namespace Sivin {
  PlayerParamsSet::PlayerParamsSet() {

  }

  PlayerParamsSet::~PlayerParamsSet() {

  }

  void PlayerParamsSet::reset() {
    timeoutMs = 15000;
  }
} // Sivin