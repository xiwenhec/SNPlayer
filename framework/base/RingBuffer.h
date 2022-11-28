
/*
 *      Copyright (C) 2015 pingkai010@gmail.com
 *
 */
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct RingBuffer_t RingBuffer;

RingBuffer *RingBufferCreate(uint32_t size);

void RingBufferSetBackSize(RingBuffer *rBuf,uint32_t size);

void RingBufferDestroy(RingBuffer *rBuf);

void RingBufferClear(RingBuffer *rBuf);

uint32_t RingBufferReadData(RingBuffer *rBuf, char *buf, uint32_t size);

uint32_t RingBufferWriteData(RingBuffer *rBuf, const char *buf, uint32_t size);

int64_t RingBufferSkipBytes(RingBuffer *rBuf, int64_t skipSize);

char *RingBufferGetBuffer(RingBuffer *rBuf);

uint32_t RingBufferGetSize(RingBuffer *rBuf);

unsigned int RingBufferGetReadPtr(RingBuffer *rBuf);

unsigned int RingBufferGetWritePtr(RingBuffer *rBuf);

uint32_t RingBufferGetAvailableReadSize(RingBuffer *rBuf);

uint32_t RingBufferGetAvailableWriteSize(RingBuffer *rBuf);
uint32_t RingBufferGetAvailableBackSize(RingBuffer *rBuf);

#ifdef __cplusplus
}
#endif

#endif

