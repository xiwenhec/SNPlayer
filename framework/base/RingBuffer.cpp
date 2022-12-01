
#include "RingBuffer.h"
#include <cstring>
#include <atomic>
#include <cassert>

RingBuffer::RingBuffer(uint32_t size) : RingBuffer(size, 0) {}

RingBuffer::RingBuffer(uint32_t size, uint32_t backSize) : mSize(size),
                                                           mBackSize(backSize),
                                                           mBackCount(0), mReadIndex(0),
                                                           mWriteIndex(0), mFillCount(0) {
    mBuffer = new char[size];
    memset(mBuffer, 0, size);
}

RingBuffer::~RingBuffer() {
    if (mBuffer != nullptr) {
        delete[] mBuffer;
        mBuffer = nullptr;
    }
}

uint32_t RingBuffer::readData(char *outBuffer, uint32_t wantSize) {
    if (wantSize > mFillCount.load()) {
        return 0;
    }

    //要读取的数据需要回环
    if (wantSize + mReadIndex > mSize) {
        unsigned int chunk = mSize - mReadIndex;
        memcpy(outBuffer, mBuffer + mReadIndex, chunk);
        memcpy(outBuffer + chunk, mBuffer, wantSize - chunk);
        mReadIndex = wantSize - chunk;
        assert(mReadIndex <= mSize);
    } else {
        memcpy(outBuffer, mBuffer + mReadIndex, wantSize);
        mReadIndex += wantSize;
        assert(mReadIndex <= mSize);
    }

    if (mReadIndex == mSize) {
        mReadIndex = 0;
    }

    mFillCount -= wantSize;
    mBackCount = std::min(mBackSize, mBackCount + wantSize);

    return wantSize;
}


uint32_t RingBuffer::writeData(const char *inputBuffer, uint32_t inputSize) {

    if (inputSize > mSize - mFillCount.load() - mBackCount.load()) {
        return 0;
    }

    if (inputSize + mWriteIndex > mSize) {
        uint32_t chunk = mSize - mWriteIndex;
        memcpy(mBuffer + mWriteIndex, inputBuffer, chunk);
        memcpy(mBuffer, inputBuffer + chunk, inputSize - chunk);
        mWriteIndex = inputSize - chunk;
    } else {
        if (mBuffer + mWriteIndex != inputBuffer) {
            memcpy(mBuffer + mWriteIndex, inputBuffer, inputSize);
        }
        mWriteIndex += inputSize;
    }

    if (mWriteIndex == mSize) {
        mWriteIndex = 0;
    }

    mFillCount += inputSize;
    return inputSize;
}

void RingBuffer::clear() {
    mReadIndex = 0;
    mWriteIndex = 0;
    mFillCount = 0;
    mBackCount = 0;
}

int64_t RingBuffer::skipBytes(int64_t skpSize) {

    int64_t skipSize = 0;

    if (skpSize < 0) {
        skipSize = -skpSize;
        //回退的长度小于(读后缓冲 backCount)长度，表示可以直接缓冲回退，不会污染写空间
        if (mBackCount.load() >= skipSize) {
            mBackCount -= skipSize;
            //当前读指针的位置小于要回推的大小，则需要回环回退
            if (mReadIndex < skipSize) {
                mReadIndex = mSize - (skipSize - mReadIndex);
                assert(mReadIndex <= mSize);
            } else { //直接回退指针
                mReadIndex -= skipSize;
                assert(mReadIndex <= mSize);
            }
            if (mReadIndex == mSize) {
                mReadIndex = 0;
            }
            //更新可读数据大小
            mFillCount += skipSize;
            return skipSize;
        }
        //不能回退
        return 0;
    }
    skipSize = skpSize;
    //向前seek，超过了可读空间大小，直接返回
    if (skipSize > mFillCount) {
        return 0;
    }

    if (skipSize + mReadIndex > mSize) {
        //前进需要回环
        uint64_t chunk = mSize - mReadIndex;
        mReadIndex = skipSize - chunk;
        assert(mReadIndex <= mSize);
    } else {
        //直接更新读指针
        mReadIndex += skipSize;
        assert(mReadIndex <= mSize);
    }
    if (mReadIndex == mSize) {
        mReadIndex = 0;
    }
    mFillCount -= skipSize;
    return skipSize;
}

uint64_t RingBuffer::getMaxReadableDataSize() const {
    return mFillCount.load();
}

uint64_t RingBuffer::getMaxWriteableDataSize() const {
    return mSize - mFillCount.load() - mBackCount.load();;
}

uint64_t RingBuffer::getMaxBackSize() const {
    return mBackCount;
}




