#include "ringbuffer.h"

RingBuffer::RingBuffer() : front(0), rear(0), bufferSize(BUFSIZE) {
    buffer = new char[BUFSIZE];
    memset(buffer, 0, BUFSIZE);

}

RingBuffer::RingBuffer(int iBufferSize) : front(0), rear(0), bufferSize(iBufferSize) {
 
    buffer = new char[iBufferSize]; 
    memset(buffer, 0, iBufferSize);
}

RingBuffer::~RingBuffer() {
    delete[] buffer;
}

void RingBuffer::Resize(int size) {
    // 기존 버퍼 크기보단 커야함.
    if (size <= bufferSize) {

        __debugbreak();
        return;

    }
    char* newBuffer = new char[size];
    memset(newBuffer, 0, size);

    // 기존 데이터 옮기기
    int useSize = GetUseSize();
    int firstChunk = min(useSize, bufferSize - front);
    memcpy(newBuffer, &buffer[front], firstChunk);
    memcpy(newBuffer + firstChunk, &buffer[0], useSize - firstChunk);

    delete[] buffer;
    buffer = newBuffer;
    bufferSize = size;
    front = 0;
    rear = useSize;
}

int RingBuffer::GetBufferSize() {
    return bufferSize;
}

int RingBuffer::GetUseSize() {
    return (rear - front + bufferSize) % bufferSize;
}

int RingBuffer::GetFreeSize() {
    return bufferSize - 1 - GetUseSize();
}

int RingBuffer::Enqueue(const char* chpData, int iSize) {
    
    if (iSize <= 0) return 0;
    if (iSize > GetFreeSize())
        return RINGBUFFER_ERROR_NOSPACE;

    int firstChunk = min(iSize, bufferSize - rear);
    memcpy(&buffer[rear], chpData, firstChunk);
    memcpy(&buffer[0], chpData + firstChunk, iSize - firstChunk);
    rear = (rear + iSize) % bufferSize;
    return iSize;
}

int RingBuffer::Dequeue(char* chpDest, int iSize) {
    
    if (iSize <= 0) return 0;
    if (iSize > GetUseSize())
        return RINGBUFFER_ERROR_NODATA;

    int firstChunk = min(iSize, bufferSize - front);
    memcpy(chpDest, &buffer[front], firstChunk);
    memcpy(chpDest + firstChunk, &buffer[0], iSize - firstChunk);
    front = (front + iSize) % bufferSize;
    return iSize;
}

int RingBuffer::Peek(char* chpDest, int iSize) {
    
    if (iSize <= 0) return 0;

    if (iSize > GetUseSize()) return RINGBUFFER_ERROR_NODATA;

    int firstChunk = min(iSize, bufferSize - front);
    memcpy(chpDest, &buffer[front], firstChunk);
    memcpy(chpDest + firstChunk, &buffer[0], iSize - firstChunk);
    // front 이동 없음
    return iSize;
}

void RingBuffer::ClearBuffer() {
    front = 0;
    rear = 0;
    memset(buffer, 0, bufferSize);
}

int RingBuffer::DirectEnqueueSize() {
    // rear부터 끊기지 않고 쓸 수 있는 길이
    if (rear >= front)
        return bufferSize - rear - (front == 0 ? 1 : 0);
    else
        return front - rear - 1;
}

int RingBuffer::DirectDequeueSize() {
    // front부터 끊기지 않고 읽을 수 있는 길이
    if (rear >= front)
        return rear - front;
    else
        return bufferSize - front;
}

/* 로컬 버퍼 안쓰기 위한 방식 */

// 인큐 후 rear 이동
int RingBuffer::MoveRear(int iSize) {
    if (iSize > GetFreeSize()) return RINGBUFFER_ERROR_NOSPACE;
    rear = (rear + iSize) % bufferSize;
    return iSize;
}
// 디큐 후 front 이동
int RingBuffer::MoveFront(int iSize) {
    if (iSize > GetUseSize()) return RINGBUFFER_ERROR_NODATA;
    front = (front + iSize) % bufferSize;
    return iSize;
}

/* 로컬 버퍼 안쓰기 위한 방식 */


char* RingBuffer::GetFrontBufferPtr() {
    return &buffer[front];
}

char* RingBuffer::GetRearBufferPtr() {
    return &buffer[rear];
}