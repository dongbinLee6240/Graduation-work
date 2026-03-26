#pragma once
#include <queue>
#include <vector>
#include <mutex> //스레드 동기화를 위한 mutex 헤더
#include <WinSock2.h>
#include "RFT_IOCP_MatchingServer.h"

#define MAX_TEAM_SIZE 2  // 디버깅 용 팀 사이즈
#define MAX_TEAMS 1      // 디버깅 용 최대 팀 수

class MatchManager
{
private:

    MatchManager();  // private 생성자
    ~MatchManager(); // private 소멸자

    std::queue<Player> playerQueue; // 대기열
    std::vector<Team> teams;        // 팀 목록
    std::mutex m_mutex;

public:
    static MatchManager& GetInstance();

    void MatchPlayers();                  // 매칭 로직
    void SendMatchCompletePackets();      // 매칭 완료 패킷 전송
    void addPlayerToQueue(const Packet& packet, SOCKET ClientSocket); // 플레이어 추가
};
