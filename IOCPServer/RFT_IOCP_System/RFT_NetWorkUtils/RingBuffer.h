#pragma once
#include <vector>
#include <cstring> // memcpy 사용을 위함

class RingBuffer
{
public:
    // 기본 버퍼 크기를 8192바이트(8KB)로 설정합니다. 필요에 따라 늘릴 수 있습니다.
    RingBuffer(size_t capacity = 8192);
    ~RingBuffer() = default;

    // 네트워크에서 받은 데이터를 버퍼에 밀어 넣습니다. (WSARecv 직후 호출)
    bool Enqueue(const char* data, size_t size);

    // 버퍼에서 데이터를 빼냅니다. 빼낸 데이터는 버퍼에서 지워집니다. (패킷 처리 시 호출)
    bool Dequeue(char* dest, size_t size);

    // 버퍼에서 데이터를 지우지 않고 읽기만 합니다. (패킷의 전체 길이를 확인할 때 호출)
    bool Peek(char* dest, size_t size) const;

    // 현재 버퍼에 쌓여있는 데이터의 크기 (처리해야 할 데이터 양)
    size_t GetUsedSize() const;

    // 현재 버퍼에 남아있는 빈 공간의 크기
    size_t GetFreeSpace() const;

    // 버퍼 초기화
    void Clear();

private:
    std::vector<char> buffer_;
    size_t capacity_;
    size_t front_;     // 읽기 포인터 (데이터를 빼갈 위치)
    size_t rear_;      // 쓰기 포인터 (데이터를 넣을 위치)
    size_t usedSize_;  // 현재 저장된 데이터 크기
};