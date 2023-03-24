#include "SnPlayer.h"
#include <cstdio>

int main() {


  printf("player test run...\n");

  static Sivin::PlayerMsg dummyMsg{};

  Sivin::SnPlayer player;
  player.setDataSource("https://player.alicdn.com/video/aliyunmedia.mp4");
  player.prepare();


  return 0;
}