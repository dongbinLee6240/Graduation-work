#include "MatchManager.h"
#include <algorithm>
#include <cstdio>

MatchManager* MatchManager::Instance = nullptr;

MatchManager::MatchManager()
{
    printf("[INFO] MatchManager가 초기화되었습니다.\n");
    while (!playerQueue.empty())
        playerQueue.pop();
    teams.clear();
}

MatchManager::~MatchManager()
{
    printf("[INFO] MatchManager가 종료되었습니다.\n");
    while (!playerQueue.empty())
        playerQueue.pop();
    teams.clear();
}

MatchManager* MatchManager::GetInstance()
{
    if (!Instance)
    {
        Instance = new MatchManager();
    }
    return Instance;
}

void MatchManager::DestroyInstance()
{
    if (Instance)
    {
        delete Instance;
        Instance = nullptr;
    }
}

void Packet::MatchCompletePacket(Packet* packet, char* buffer)
{
    PacketMaker packetmaker;
    ZeroMemory(buffer, BUFSIZE);

    packet->headercode = MATCH_COMPLETE;
    packet->data = "MatchComplete";
    packet->end = 0xFFFF;
    packet->length = sizeof(packet->length) + sizeof(packet->headercode)
        + strlen(packet->data) + sizeof(packet->end);

    packetmaker.serialize(packet, buffer);
    printf("[DEBUG] MatchCompletePacket - Length: %zu, Data: %s, End: %04X\n",
        strlen(packet->data), packet->data, packet->end);
}

void MatchManager::addPlayerToQueue(const Packet& packet, SOCKET ClientSocket)
{
    Player player;
    player.cid = packet.data;  // 패킷에서 UID 추출
    player.socket = ClientSocket;

    playerQueue.push(player);
    printf("[INFO] Player CID %s가 대기열에 추가되었습니다. 현재 대기열 크기: %zu\n", player.cid.c_str(), playerQueue.size());

    // 현재 저장된 모든 플레이어의 소켓 출력
    printf("[DEBUG] 현재 대기열에 있는 모든 플레이어 소켓 정보:\n");
    std::queue<Player> tempQueue = playerQueue; // 대기열 복사본
    while (!tempQueue.empty())
    {
        Player tempPlayer = tempQueue.front();
        tempQueue.pop();
        printf(" - Player CID: %s, Socket: %lld\n", tempPlayer.cid.c_str(), (long long)tempPlayer.socket);
    }
}

void MatchManager::MatchPlayers()
{
    while (!playerQueue.empty())
    {
        Player player = playerQueue.front();
        playerQueue.pop();

        bool addedToTeam = false;

        // 기존 팀에 추가
        for (auto& team : teams)
        {
            if (!team.isComplete())
            {
                team.players.push_back(player);
                addedToTeam = true;
                break;
            }
        }

        // 새로운 팀 생성
        if (!addedToTeam)
        {
            Team newTeam;
            newTeam.players.push_back(player);
            teams.push_back(newTeam);
        }

        // 팀 상태 디버깅 로그
        printf("[DEBUG] 현재 대기열 크기: %zu\n", playerQueue.size());
        for (const auto& team : teams)
        {
            printf("[DEBUG] 팀 크기: %zu, 플레이어: ", team.players.size());
            for (const auto& p : team.players)
            {
                printf("%s ", p.cid.c_str());
            }
            printf("\n");
        }

        // 모든 팀이 완성되었는지 확인
        if (teams.size() <= MAX_TEAMS &&
            std::all_of(teams.begin(), teams.end(), [](const Team& team) { return team.isComplete(); }))
        {
            printf("[INFO] 모든 팀이 완성되었습니다. 매칭 완료 패킷 전송.\n");
            SendMatchCompletePackets();
        }
    }
}

void MatchManager::SendMatchCompletePackets()
{
    for (const auto& team : teams)
    {
        for (const auto& player : team.players)
        {
            char sendBuffer[BUFSIZE];
            Packet matchCompletePacket;

            matchCompletePacket.MatchCompletePacket(&matchCompletePacket, sendBuffer);

            SOCKETINFO* pSocketInfo = new SOCKETINFO();
            ZeroMemory(pSocketInfo, sizeof(SOCKETINFO)); // 초기화
            pSocketInfo->socket = player.socket; //이부분에서 플레이어정보를 활요
            printf("[DEBUG] Sending to Socket: %lld, Data: MatchComplete\n", (long long)player.socket);
            // 데이터 버퍼 설정
            pSocketInfo->dataBuf.buf = sendBuffer;
            pSocketInfo->dataBuf.len = matchCompletePacket.length;

            DWORD sendBytes;
            //Send
            int result = WSASend(
                player.socket,
                &(pSocketInfo->dataBuf),
                1,
                &sendBytes,
                0,
                NULL,
                NULL
            );

            if (result == SOCKET_ERROR)
            {
                printf("[ERROR] 매칭 완료 패킷 전송 실패 - WSA Error: %d\n", WSAGetLastError());
            }
            else
            {
                printf("[INFO] 메시지 송신 - Bytes: [%d], HeaderCode: [%d], Msg: [%s]\n",
                    sendBytes,
                    matchCompletePacket.headercode,
                    sendBuffer);
            }

            delete pSocketInfo; // 메모리 해제
        }
    }
}
