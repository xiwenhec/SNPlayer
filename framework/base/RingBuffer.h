
#ifndef FRAMKEWORK_BASE_RINGBUFFER_H
#define FRAMKEWORK_BASE_RINGBUFFER_H

#include <cstdint>
#include <atomic>
#include <memory>

class RingBuffer {
public:
  explicit RingBuffer(int64_t size);

  RingBuffer(int64_t size, int64_t backSize);

  ~RingBuffer();

  [[maybe_unused]] void setBackSize(int64_t size) {
    mBackSize = size;
  }

  int64_t readData(char *outBuffer, int64_t size);

  int64_t writeData(const char *inputBuffer, int64_t size);

  // 成功返回跳过的字节数，失败返回0
  int64_t skipBytes(int64_t skipSize);

  [[nodiscard]] int64_t getReadableSize() const;

  [[nodiscard]] int64_t getWriteableSize() const;

  [[maybe_unused]] [[nodiscard]] int64_t getBackSize() const;

  void clear();

private:
  char *mBuffer;
  int64_t mSize;
  int64_t mBackSize;
  unsigned int mReadIndex;
  unsigned int mWriteIndex;
  std::atomic<int64_t> mFillCount;
  // 为了能够回退已经读取过的数据而保留的长度，
  // 写指针不能占用这部分空间
  std::atomic<int64_t> mBackCount;

}; // RingBuffer

#endif // FRAMKEWORK_BASE_RINGBUFFER_H
