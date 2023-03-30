#ifndef SN_FRAMEWORK_SNCONCURQUEUE_H
#define SN_FRAMEWORK_SNCONCURQUEUE_H

#include "utils/ReaderWriterqueue.h"
namespace Sivin {
  template<typename T>
  class SNConcurQueue : public moodycamel::ReaderWriterQueue<T> {
  public:
    explicit SNConcurQueue(size_t size = 15) : moodycamel::ReaderWriterQueue<T>(size) {
    }
    ~SNConcurQueue() = default;

  public:
    size_t size() const {
      return this->size_approx();
    }
    bool empty() const {
      return this->size_approx() == 0;
    }
  };

}// namespace Sivin

#endif