#pragma once
#include <algorithm>
#include <Windows.h>

#define BUFSIZE 50000
#define RINGBUFFER_ERROR_NOSPACE -1  // 인큐: 남은 공간 부족
#define RINGBUFFER_ERROR_NODATA  -2  // 디큐: 데이터 부족
class RingBuffer {
public:
	RingBuffer();
	~RingBuffer();
	RingBuffer(int iBufferSize);
	void Resize(int size);

	int GetBufferSize();
	int GetUseSize();
	int GetFreeSize();

	int Enqueue(const char* chpData, int iSize);
	int Dequeue(char* chpDest, int iSize);
	int Peek(char* chpDest, int iSize);

	void ClearBuffer();

	int DirectEnqueueSize();
	int DirectDequeueSize();

	int MoveRear(int iSize);
	int MoveFront(int iSize);

	char* GetFrontBufferPtr();
	char* GetRearBufferPtr();

public:
	int front;
	int rear;
	int bufferSize;
	char* buffer;
};