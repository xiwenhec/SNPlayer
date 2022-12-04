
#ifndef SIVINPLAYER_RINGBUFFER_H
#define SIVINPLAYER_RINGBUFFER_H

#include <cstdint>
#include <atomic>
#include <memory>

class RingBuffer {
public:
    explicit RingBuffer(uint64_t size);

    RingBuffer(uint64_t size, uint64_t backSize);

    ~RingBuffer();

    [[maybe_unused]] void setBackSize(uint32_t size) { mBackSize = size; }

    uint64_t readData(char *outBuffer, uint64_t size);

    uint64_t writeData(const char *inputBuffer, uint64_t size);

    [[maybe_unused]] int64_t skipBytes(int64_t skipSize);

    [[nodiscard]] uint64_t getMaxReadableDataSize() const;

    [[nodiscard]] uint64_t getMaxWriteableDataSize() const;

    [[maybe_unused]] [[nodiscard]] uint64_t getMaxBackSize() const;

    void clear();

private:
    char *mBuffer;
    uint64_t mSize;
    uint64_t mBackSize;
    unsigned int mReadIndex;
    unsigned int mWriteIndex;
    std::atomic<uint64_t> mFillCount;
    //为了能够回退已经读取过的数据而保留的长度，
    //写指针不能占用这部分空间
    std::atomic<uint64_t> mBackCount;
};

#endif
