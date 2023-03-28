#ifndef SIVINPLAYER_REFERCLOCK_H
#define SIVINPLAYER_REFERCLOCK_H
#include <cstdint>
namespace Sivin {

  class ReferClock {
  
   public:
        ReferClock() = default;

        ~ReferClock() = default;

        void start();

        void pause();

        void setTime(int64_t time);

        int64_t GetTime();

        void SetScale(float scale);

        float GetScale();

        // void setReferenceClock(get_clock getClock, void *arg);

        void reset();

        bool haveMaster();

        bool isMasterValid();

    private:
        // af_scalable_clock mClock;
        // get_clock mGetClock{nullptr};
        // void *mClockArg{nullptr};
  
  
  };


}// namespace Sivin

#endif//SIVINPLAYER_REFERCLOCK_H