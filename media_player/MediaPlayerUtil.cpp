#include "MediaPlayerUtil.h"
#include "utils/SNTimer.h"
#include <cstdint>


namespace Sivin {

  void MediaPlayerUtil::notifyRead(PacketReadEvent event, uint64_t size) {
    switch (event) {
      case PacketReadEvent::EVENT_AGAIN:
        mReadTimes++;
        break;
      case PacketReadEvent::EVENT_GOT:
        mReadGotIndex++;
        mReadGotSize += size;
        break;
      case PacketReadEvent::EVENT_LOOP:
        mReadLoopIndex++;
        break;
      case PacketReadEvent::EVENT_TIME_OUT:
        mReadTimeOutIndex++;
        break;
      case PacketReadEvent::EVENT_NETWORK:
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