#include "SNActiveDecoder.h"
#include "utils/SNThread.h"
#include "utils/SNTimer.h"

#define MAX_OUTPUT_SIZE 16
#define MAX_INPUT_SIZE (20 * 30)

namespace Sivin {

  SNActiveDecoder::SNActiveDecoder() : mInputQueue(MAX_INPUT_SIZE), mOutputQueue(MAX_OUTPUT_SIZE) {
  }

  SNActiveDecoder::~SNActiveDecoder() {
  }

  int SNActiveDecoder::open(const std::unique_ptr<SNStreamInfo> &streamInfo, void *surface, uint64_t flags) {
    int ret = initDecoder(streamInfo, surface, flags);
    if (ret < 0) {
      close();
      return ret;
    }

    mRunning = true;
    mDeocdeThread = MAKE_UNIQUE_THREAD(decodeFunc, "decodeThread");
    mDeocdeThread->start();

    return ret;
  }


  int SNActiveDecoder::decodeFunc() {
    if (mDecoderEOS) {
      SNTimer::sleepUs(10000);
      return 0;
    }


    return 0;
  }


  void SNActiveDecoder::close() {
  }


}// namespace Sivin