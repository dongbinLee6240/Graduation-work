#pragma once
#include <queue>
#include <vector>
#include <WinSock2.h>
#include "RFT_IOCP_MatchingServer.h"

#define MAX_TEAM_SIZE 2  // ����� �� �� ������
#define MAX_TEAMS 1      // ����� �� �ִ� �� ��

class MatchManager
{
private:
    static MatchManager* Instance;

    MatchManager();  // private ������
    ~MatchManager(); // private �Ҹ���

    std::queue<Player> playerQueue; // ��⿭
    std::vector<Team> teams;        // �� ���

public:
    static MatchManager* GetInstance();
    static void DestroyInstance();

    void MatchPlayers();                  // ��Ī ����
    void SendMatchCompletePackets();      // ��Ī �Ϸ� ��Ŷ ����
    void addPlayerToQueue(const Packet& packet, SOCKET ClientSocket); // �÷��̾� �߰�
};
