#include "MediaPlayerDef.h"
#include "SnPlayer.h"
#include "utils/SNThread.h"
#include "utils/SNTimer.h"
#include <cstdio>

using namespace Sivin;

void onPrepared(void *userData) {
}

int main() {
  printf("player test run...\n");

  PlayerListener mListener{};
  if (mListener.onPreparedCallback == nullptr) {
    mListener.onPreparedCallback = onPrepared;
  }
  // SnPlayer player;
  // player.setDataSource("https://player.alicdn.com/video/aliyunmedia.mp4");
  // player.prepare();

  SNTimer::sleepMs(600 * 1000);
  return 0;
}