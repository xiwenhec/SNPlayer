#ifndef SN_FRAMEWORK_SNSPSCQUEUE_H
#define SN_FRAMEWORK_SNSPSCQUEUE_H
#include <boost/lockfree/spsc_queue.hpp>
namespace Sivin {

  template<typename T>
  class SNSpscQueue : public boost::lockfree::spsc_queue<T> {
  public:
    SNSpscQueue(int size) : boost::lockfree::spsc_queue<T>(size){};
    size_t size() {
      return this->read_available();
    }
  };

}// namespace Sivin

#endif