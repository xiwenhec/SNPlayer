
#ifndef SIVINPLAYER_RINGBUFFER_H
#define SIVINPLAYER_RINGBUFFER_H

#include <cstdint>
#include <atomic>
#include <memory>

//#ifdef __cplusplus
//extern "C" {
//#endif
//struct RingBuffer_t;
//
//RingBuffer_t *RingBufferCreate(uint32_t size);
//
//void RingBufferSetBackSize(RingBuffer_t *rBuf, uint32_t size);
//
//void RingBufferDestroy(RingBuffer_t *rBuf);
//
//void RingBufferClear(RingBuffer_t *rBuf);
//
//uint32_t RingBufferReadData(RingBuffer_t *rBuf, char *buf, uint32_t size);
//
//uint32_t RingBufferWriteData(RingBuffer_t *rBuf, const char *buf, uint32_t size);
//
//int64_t RingBufferSkipBytes(RingBuffer_t *rBuf, int64_t skipSize);
//
//char *RingBufferGetBuffer(RingBuffer_t *rBuf);
//
//uint32_t RingBufferGetSize(RingBuffer_t *rBuf);
//
//unsigned int RingBufferGetReadPtr(RingBuffer_t *rBuf);
//
//unsigned int RingBufferGetWritePtr(RingBuffer_t *rBuf);
//
//uint32_t RingBufferGetAvailableReadSize(RingBuffer_t *rBuf);
//
//uint32_t RingBufferGetAvailableWriteSize(RingBuffer_t *rBuf);
//uint32_t RingBufferGetAvailableBackSize(RingBuffer_t *rBuf);
//
//#ifdef __cplusplus
//}
//#endif
//


class RingBuffer {
public:
    explicit RingBuffer(uint32_t size);

    RingBuffer(uint32_t size, uint32_t backSize);

    ~RingBuffer();

    void setBackSize(uint32_t size) { mBackSize = size; }

    uint32_t readData(char *outBuffer, uint32_t size);

    uint32_t writeData(const char *inputBuffer, uint32_t size);

    int64_t skipBytes(int64_t skipSize);

    uint64_t getMaxReadableDataSize() const;

    uint64_t getMaxWriteableDataSize() const;

    uint64_t getMaxBackSize() const;

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
