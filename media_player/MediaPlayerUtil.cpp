#include "MediaPlayerUtil.h"
#include "utils/SNTimer.h"
#include <cstdint>


namespace Sivin {

  void MediaPlayerUtil::notifyRead(PacketReadEvent event, uint64_t size) {
    switch (event) {
      case PacketReadEvent::AGAIN:
        mReadTimes++;
        break;
      case PacketReadEvent::GOT:
        mReadGotIndex++;
        mReadGotSize += size;
        break;
      case PacketReadEvent::LOOP:
        mReadLoopIndex++;
        break;
      case PacketReadEvent::TIME_OUT:
        mReadTimeOutIndex++;
        break;
      case PacketReadEvent::NETWORK:
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