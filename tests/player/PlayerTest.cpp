#include "MediaPlayerDef.h"
#include "SnPlayer.h"
#include "utils/SNThread.h"
#include "utils/SNTimer.h"
#include <cstdio>
#include <sys/sysinfo.h>
#include <utils/os/SNSysInfoUtil.h>

using namespace Sivin;

void test() {
  // struct sysinfo info;
  // sysinfo(&info);
  // printf("%lu %lu\n", info.totalram / (1024 *1024), (info.bufferram + info.freeram) / (1024 *1024));

  // SNSysInfo snInfo;
  // SNSysInfoUtil::getSystemMemoryInfo(&snInfo);
  // printf("%lu %lu\n", snInfo.totalram/1024, snInfo.availableram/1024);
}

static void onPrepared(void *userData) {
  SnPlayer *player = static_cast<SnPlayer *>(userData);
  player->start();
}

int main() {
  test();

  // printf("player test run...\n");

  // SnPlayer *player = new SnPlayer();

  // PlayerListener listener{};
  // listener.userData = player;
  // listener.onPreparedCallback = onPrepared;
  // player->setListener(listener);
  // player->setDataSource("https://player.alicdn.com/video/aliyunmedia.mp4");
  // player->prepare();

  // SNTimer::sleepMs(600 * 1000);
  // delete player;
  return 0;
}