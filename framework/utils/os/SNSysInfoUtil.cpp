#include "SNSysInfoUtil.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <linux/sysinfo.h>
#include <strings.h>
#include <sys/sysinfo.h>

namespace Sivin {

  static FILE *gProcMeminfofp = nullptr;

  int SNSysInfoUtil::getSystemMemoryInfo(SNSysInfo *snInfo) {
    if (snInfo == nullptr) {
      return -1;
    }

    memset(snInfo, 0, sizeof(SNSysInfo));

    if (!gProcMeminfofp && (gProcMeminfofp = fopen("/proc/meminfo", "r")) == nullptr) {
      struct sysinfo info;
      sysinfo(&info);
      snInfo->totalram = static_cast<uint64_t>((info.totalram * info.mem_unit)) / 1024;
      snInfo->availableram = static_cast<uint64_t>((info.freeram + info.bufferram) * info.mem_unit) / 1024;
      snInfo->freeram = static_cast<uint64_t>((info.freeram * info.mem_unit)) / 1024;
    } else {
      uint64_t val = 0;
      char key[32];
      bzero(key, 32);
      while (fscanf(gProcMeminfofp, "%31s %lu%*[^\n]\n", key, &val) != EOF) {
        if (strncmp("MemTotal:", key, 9) == 0) {
          snInfo->totalram = val;
        } else if (strncmp("MemFree:", key, 8) == 0) {
          snInfo->freeram = val;
          snInfo->availableram += val;
        } else if (strncmp("Buffers:", key, 8) == 0) {
          snInfo->availableram += val;
        } else if (strncmp("MemAvailable:", key, 13)) {
          //这里没有使用MemAvailable，而是采用free+buffer的形式，是保守策略
          //MemAvailable里的值要大于free+buffer的值
        }
      }
    }
    return 0;
  }


}// namespace Sivin
