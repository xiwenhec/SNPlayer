
/*
 *      Copyright (C) 2015 pingkai010@gmail.com
 *
 */

#include "RingBuffer.h"
#include <cstdlib>
#include <cstring>
#include <atomic>
#include <algorithm>
#include <cassert>

using namespace std;

typedef struct RingBuffer_t {
    char *m_buffer;
    unsigned int m_size;
    unsigned int m_readPtr;
    unsigned int m_writePtr;
    atomic<uint32_t> m_fillCount;
    unsigned int m_back_size;
    atomic<uint32_t> m_backCount;
} RingBuffer;

RingBuffer *RingBufferCreate(uint32_t size) {
    RingBuffer *rBuf;
    char *buf = (char *) malloc(size);

    if (!buf) {
        return nullptr;
    }

    rBuf = (RingBuffer *) malloc(sizeof(RingBuffer));
    memset(rBuf, 0, sizeof(RingBuffer));
    rBuf->m_size = size;
    rBuf->m_buffer = buf;
    rBuf->m_back_size = 0;
    rBuf->m_backCount = 0;
    return rBuf;
}

void RingBufferSetBackSize(RingBuffer *rBuf, uint32_t size) {
    rBuf->m_back_size = size;
}

void RingBufferDestroy(RingBuffer *rBuf) {
    if (rBuf->m_buffer != nullptr) {
        free(rBuf->m_buffer);
        rBuf->m_buffer = nullptr;
    }

    free(rBuf);
}

void RingBufferClear(RingBuffer *rBuf) {
    rBuf->m_readPtr = 0;
    rBuf->m_writePtr = 0;
    rBuf->m_fillCount = 0;
    rBuf->m_backCount = 0;
}

uint32_t RingBufferReadData(RingBuffer *rBuf, char *buf, uint32_t size) {
    if (size > rBuf->m_fillCount.load()) {
        return 0;
    }

    if (size + rBuf->m_readPtr > rBuf->m_size) {
        unsigned int chunk = rBuf->m_size - rBuf->m_readPtr;
        memcpy(buf, rBuf->m_buffer + rBuf->m_readPtr, chunk);
        memcpy(buf + chunk, rBuf->m_buffer, size - chunk);
        rBuf->m_readPtr = size - chunk;
        assert(rBuf->m_readPtr <= rBuf->m_size);
    } else {
        memcpy(buf, rBuf->m_buffer + rBuf->m_readPtr, size);
        rBuf->m_readPtr += size;
        assert(rBuf->m_readPtr <= rBuf->m_size);
    }

    if (rBuf->m_readPtr == rBuf->m_size) {
        rBuf->m_readPtr = 0;
    }

    rBuf->m_fillCount -= size;
    rBuf->m_backCount = std::min(rBuf->m_back_size, rBuf->m_backCount + size);
    return size;
}

uint32_t RingBufferWriteData(RingBuffer *rBuf, const char *buf, uint32_t size) {
    if (size > rBuf->m_size - rBuf->m_fillCount.load() - rBuf->m_backCount.load()) {
        return 0;
    }

    //剩余空间小于要写入的数据大小，需要回环
    if (size + rBuf->m_writePtr > rBuf->m_size) {
        //获取写索引到结束位置的空间
        unsigned int chunk = rBuf->m_size - rBuf->m_writePtr;
        //拷贝部分数据，从写索引开始位置处写入
        memcpy(rBuf->m_buffer + rBuf->m_writePtr, buf, chunk);
        //回环写入
        memcpy(rBuf->m_buffer, buf + chunk, size - chunk);
        //更新写索引
        rBuf->m_writePtr = size - chunk;
    } else {
        if ((rBuf->m_buffer + rBuf->m_writePtr) != buf) {
            memcpy(rBuf->m_buffer + rBuf->m_writePtr, buf, size);
        }

        rBuf->m_writePtr += size;
    }

    if (rBuf->m_writePtr == rBuf->m_size) {
        rBuf->m_writePtr = 0;
    }

    rBuf->m_fillCount += size;
    return size;
}

int64_t RingBufferSkipBytes(RingBuffer *rBuf, int64_t skipSize) {
    int64_t size;

    if (skipSize < 0) {
        size = -skipSize;
        //    AF_LOGI("skip size is %d\n", skipSize);

        if (rBuf->m_backCount.load() >= size) {
            rBuf->m_backCount -= size;

            if (rBuf->m_readPtr < size) {
                rBuf->m_readPtr = rBuf->m_size - (size - rBuf->m_readPtr);
                assert(rBuf->m_readPtr <= rBuf->m_size);
            } else {
                rBuf->m_readPtr -= size;
                assert(rBuf->m_readPtr <= rBuf->m_size);
            }

            if (rBuf->m_readPtr == rBuf->m_size) {
                rBuf->m_readPtr = 0;
            }

            rBuf->m_fillCount += size;
            return skipSize;
        }

        return 0;
    }

    size = skipSize;

    if (size > rBuf->m_fillCount) {
        return 0;
    }

    if (size + rBuf->m_readPtr > rBuf->m_size) {
        unsigned int chunk = rBuf->m_size - rBuf->m_readPtr;
        rBuf->m_readPtr = size - chunk;
        assert(rBuf->m_readPtr <= rBuf->m_size);
    } else {
        rBuf->m_readPtr += size;
        assert(rBuf->m_readPtr <= rBuf->m_size);
    }

    if (rBuf->m_readPtr == rBuf->m_size) {
        rBuf->m_readPtr = 0;
    }

    rBuf->m_fillCount -= size;
    return skipSize;
}

char *RingBufferGetBuffer(RingBuffer *rBuf) {
    return rBuf->m_buffer;
}

uint32_t RingBufferGetSize(RingBuffer *rBuf) {
    return rBuf->m_size;
}

unsigned int RingBufferGetReadPtr(RingBuffer *rBuf) {
    return rBuf->m_readPtr;
}

unsigned int RingBufferGetWritePtr(RingBuffer *rBuf) {
    return rBuf->m_writePtr;
}

uint32_t RingBufferGetMaxReadSize(RingBuffer *rBuf) {
    return rBuf->m_fillCount.load();
}

uint32_t RingBufferGetMaxWriteSize(RingBuffer *rBuf) {
    return rBuf->m_size - rBuf->m_fillCount.load() - rBuf->m_backCount.load();
}

uint32_t RingBufferGetMaxBackSize(RingBuffer *rBuf) {
    return rBuf->m_backCount;
}