#include "RingBuffer.h"

//생성자
RingBuffer::RingBuffer(size_t capacity)
    : capacity_(capacity), front_(0), rear_(0), usedSize_(0)
{
    buffer_.resize(capacity_);
}

// 네트워크에서 받은 데이터를 버퍼에 밀어 넣습니다.
bool RingBuffer::Enqueue(const char* data, size_t size)
{
    // 남은 공간보다 넣으려는 데이터가 크면 실패 (버퍼 오버플로우 방지)
    if (GetFreeSpace() < size) return false;

    // 쓰기 포인터(rear)부터 버퍼 끝까지의 남은 직선 공간
    size_t rightSpace = capacity_ - rear_;

    if (size <= rightSpace)
    {
        // 남은 직선 공간 안에 다 들어간다면 한 번에 복사
        std::memcpy(&buffer_[rear_], data, size);
    }
    else
    {
        // 버퍼 끝을 넘어간다면, 잘라서 두 번에 나눠 복사 (원형으로 돌아감)
        std::memcpy(&buffer_[rear_], data, rightSpace);
        std::memcpy(&buffer_[0], data + rightSpace, size - rightSpace);
    }

    // 쓰기 포인터 이동 및 사용량 증가
    rear_ = (rear_ + size) % capacity_;
    usedSize_ += size;
    return true;
}

// 버퍼에서 데이터를 빼냅니다.
bool RingBuffer::Dequeue(char* dest, size_t size)
{
    // 빼내려는 데이터보다 쌓인 데이터가 적으면 실패
    if (usedSize_ < size) return false;

    // Peek를 이용해 먼저 데이터를 복사해 옴
    Peek(dest, size);

    // 읽기 포인터 이동 및 사용량 감소 (실제로 데이터를 빼냄)
    front_ = (front_ + size) % capacity_;
    usedSize_ -= size;
    return true;
}

// 버퍼에서 데이터를 지우지 않고 읽기만 합니다.
bool RingBuffer::Peek(char* dest, size_t size) const
{
    // 읽어보려는 데이터보다 쌓인 데이터가 적으면 실패
    if (usedSize_ < size) return false;

    // 읽기 포인터(front)부터 버퍼 끝까지의 남은 직선 공간
    size_t rightSpace = capacity_ - front_;

    if (size <= rightSpace)
    {
        // 한 번에 읽을 수 있는 경우
        std::memcpy(dest, &buffer_[front_], size);
    }
    else
    {
        // 끝을 넘어가서 앞부분까지 이어서 읽어야 하는 경우
        std::memcpy(dest, &buffer_[front_], rightSpace);
        std::memcpy(dest + rightSpace, &buffer_[0], size - rightSpace);
    }

    return true;
}

// 현재 버퍼에 쌓여있는 데이터의 크기
size_t RingBuffer::GetUsedSize() const
{
    return usedSize_;
}

// 현재 버퍼에 남아있는 빈 공간의 크기
size_t RingBuffer::GetFreeSpace() const
{
    return capacity_ - usedSize_;
}

void RingBuffer::Clear()
{
    front_ = 0;
    rear_ = 0;
    usedSize_ = 0;
}