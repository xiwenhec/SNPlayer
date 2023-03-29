#ifndef SN_FRAMEWORK_SNSYSNINFOUTIL_H
#define SN_FRAMEWORK_SNSYSNINFOUTIL_H
#include <cstdint>
namespace Sivin {
  struct SNSysInfo {
    uint64_t totalram;//单位kb
    uint64_t availableram;
    uint64_t freeram;
  };

  class SNSysInfoUtil {
  public:
    static int getSystemMemoryInfo(SNSysInfo *info);
  };


}// namespace Sivin

#endif//SN_FRAMEWORK_SNSYSNINFOUTIL_H