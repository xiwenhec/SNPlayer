#include "PlayerStatistic.h"
#include "utils/SNTimer.h"
#include <cstdint>


namespace Sivin {

  void PlayerStatistic::stat(StatisticEvent event, uint64_t size) {
    switch (event) {
      case StatisticEvent::LOOP:
      case StatisticEvent::GOT_PACKET:
        mReadGotSize += size;
        break;
      case StatisticEvent::NETWORK:
        mReadGotSize += size;
        break;
    }

    int64_t startTime = SNTimer::getSteadyTimeUs();
    if (mLastReadTime == 0) {
      mLastReadTime = startTime;
    } else {
      float passTime_s = float(startTime - mLastReadTime) / 1000000;
      if (passTime_s > 1.0) {
        mCurrentDownloadSpeed = (float) mReadGotSize / passTime_s;
        mLastReadTime = startTime;
        mReadGotSize = 0;
      }
    }
  }


}// namespace Sivin