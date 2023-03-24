#include "SnPlayer.h"
#include "utils/SNThread.h"
#include "utils/SNTimer.h"
#include <cstdio>

using namespace Sivin;

int main() {


  printf("player test run...\n");

  static PlayerMsg dummyMsg{};
  SnPlayer player;
  player.setDataSource("https://player.alicdn.com/video/aliyunmedia.mp4");
  player.prepare();

  SNTimer::sleepMs(600 * 1000);

  return 0;
}