#pragma once
#include <queue>
#include <mutex>
#include <WinSock2.h>
#include <cstdint>

// [신규 추가] 상호 참조(Circular Include)를 막기 위한 전방 선언
class IOCompletionPort;

struct MatchPlayer 
{
    int32_t playerId;
    SOCKET  socket;
};

class MatchManager 
{
public:
    static MatchManager& GetInstance() 
    {
        static MatchManager instance;
        return instance;
    }

    // [수정됨] 매개변수로 서버 객체의 포인터를 받습니다!
    void addPlayerToQueue(int32_t playerId, SOCKET clientSocket, IOCompletionPort* pServer);

private:
    MatchManager() = default;
    ~MatchManager() = default;
    MatchManager(const MatchManager&) = delete;
    MatchManager& operator=(const MatchManager&) = delete;

    std::queue<MatchPlayer> waitingQueue;
    std::mutex queueMutex;
};