#include "MediaPlayerDef.h"
#include "SnPlayer.h"
#include "utils/SNThread.h"
#include "utils/SNTimer.h"
#include <cstdio>

using namespace Sivin;

static void onPrepared(void *userData) {
  SnPlayer *player = static_cast<SnPlayer *>(userData);
  player->start();
}

int main() {
  printf("player test run...\n");

  SnPlayer *player = new SnPlayer();

  PlayerListener listener{};
  listener.userData = player;
  listener.onPreparedCallback = onPrepared;
  player->setListener(listener);
  player->setDataSource("https://player.alicdn.com/video/aliyunmedia.mp4");
  player->prepare();

  SNTimer::sleepMs(600 * 1000);
  delete player;
  return 0;
}