#include "MatchManager.h"
#include "RFT_IOCP_MatchingServer.h" // SendPacket을 쓰기 위해 서버 헤더 포함
#include "RFT_PacketDefinitions.h"   // Protobuf 메시지 포함
#include <iostream>

void MatchManager::addPlayerToQueue(int32_t playerId, SOCKET clientSocket, IOCompletionPort* pServer)
{
    std::lock_guard<std::mutex> lock(queueMutex);

    waitingQueue.push({ playerId, clientSocket });
    printf_s("[MatchManager] 대기열 추가 완료. (현재 대기 인원: %zu명)\n", waitingQueue.size());

    if (waitingQueue.size() >= 2)
    {
        MatchPlayer player1 = waitingQueue.front(); waitingQueue.pop();
        MatchPlayer player2 = waitingQueue.front(); waitingQueue.pop();

        printf_s("[MatchManager] 매칭 성사! (P1: %d vs P2: %d)\n", player1.playerId, player2.playerId);

        // 1. 매칭 성공 패킷 내용물 생성
        Protocol::S2C_MatchSuccess res;
        // 나중에는 여기서 AWS 등에서 할당받은 실제 게임 서버 IP를 동적으로 세팅해야 합니다!
        res.set_ds_ip("127.0.0.1");
        res.set_ds_port(7777);

        // 2. 두 유저에게 각자 패킷 발사!
        if (pServer != nullptr) {
            pServer->SendPacket(player1.socket, Protocol::RES_MATCH_SUCCESS, res);
            pServer->SendPacket(player2.socket, Protocol::RES_MATCH_SUCCESS, res);
            printf_s("[MatchManager] 두 유저에게 매칭 성공 패킷(RES_MATCH_SUCCESS) 전송 완료!\n");
        }
    }
}