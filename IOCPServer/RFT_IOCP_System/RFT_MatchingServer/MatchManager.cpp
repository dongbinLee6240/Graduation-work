#include "MatchManager.h"
#include <algorithm>
#include <cstdio>

MatchManager::MatchManager()
{
    printf("[INFO] MatchManager가 초기화되었습니다.\n");
}

MatchManager::~MatchManager()
{
    printf("[INFO] MatchManager가 종료되었습니다.\n");
}

MatchManager& MatchManager::GetInstance() //Meyers' Singleton패턴 참고
{
    static MatchManager instance;
    return instance;
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

    //함수 내에서 큐에 접근하기 전 Lock을 건다.
    std::lock_guard<std::mutex> lock(m_mutex);

    playerQueue.push(player);
    printf("[INFO] Player CID %s가 대기열에 추가되었습니다. 현재 대기열 크기: %zu\n", player.cid.c_str(), playerQueue.size());

}

void MatchManager::MatchPlayers()
{
    // 매칭 로직(큐와 벡터 수정) 전체를 보호
    std::lock_guard<std::mutex> lock(m_mutex);

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
            // 참고: SendMatchCompletePackets 함수도 m_mutex를 사용한다면 여기서 호출할 때
            // 데드락(Deadlock)에 주의해야 합니다. lock_guard는 재귀적이지 않으므로
            // SendMatchCompletePackets 내부에는 lock_guard를 빼거나 recursive_mutex를 써야 합니다.
            // 여기서는 Send 함수가 이 함수 안에서 불리므로 Send 함수에는 lock을 걸지 않았습니다.
        }
    }
}

void MatchManager::SendMatchCompletePackets()
{
    for (const auto& team : teams)
    {
        for (const auto& player : team.players)
        {
            
            //WSASend를 위한 동적 할당. 워커 스레드에서 해제될 예정.
            SOCKETINFO* pSendInfo = new SOCKETINFO();
            ZeroMemory(pSendInfo, sizeof(SOCKETINFO)); // 초기화

            pSendInfo->socket = player.socket; //이부분에서 플레이어정보를 활요
            pSendInfo->io_type = IO_SEND; //이 소켓 작업은 send용으로 정의
            
            // 데이터 버퍼 설정
            /*pSendInfo->dataBuf.buf = sendBuffer;
            pSendInfo->dataBuf.len = matchCompletePacket.length;*/

            DWORD sendBytes;
            //비동기 송신
            int result = WSASend(
                player.socket,
                &(pSendInfo->dataBuf),
                1,
                &sendBytes,
                0,
                &(pSendInfo->overlapped),
                NULL
            );

            if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
            {
                printf("[ERROR] 매칭 완료 패킷 전송 실패 - WSA Error: %d\n", WSAGetLastError());
                delete pSendInfo; // 에러가 났을 때만 즉시 해제
            }
            
        }
    }
}
