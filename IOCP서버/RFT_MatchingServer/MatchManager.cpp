#include "MatchManager.h"
#include <algorithm>
#include <cstdio>

MatchManager* MatchManager::Instance = nullptr;

MatchManager::MatchManager()
{
    printf("[INFO] MatchManager�� �ʱ�ȭ�Ǿ����ϴ�.\n");
    while (!playerQueue.empty())
        playerQueue.pop();
    teams.clear();
}

MatchManager::~MatchManager()
{
    printf("[INFO] MatchManager�� ����Ǿ����ϴ�.\n");
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
    player.cid = packet.data;  // ��Ŷ���� UID ����
    player.socket = ClientSocket;

    playerQueue.push(player);
    printf("[INFO] Player CID %s�� ��⿭�� �߰��Ǿ����ϴ�. ���� ��⿭ ũ��: %zu\n", player.cid.c_str(), playerQueue.size());

    // ���� ����� ��� �÷��̾��� ���� ���
    printf("[DEBUG] ���� ��⿭�� �ִ� ��� �÷��̾� ���� ����:\n");
    std::queue<Player> tempQueue = playerQueue; // ��⿭ ���纻
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

        // ���� ���� �߰�
        for (auto& team : teams)
        {
            if (!team.isComplete())
            {
                team.players.push_back(player);
                addedToTeam = true;
                break;
            }
        }

        // ���ο� �� ����
        if (!addedToTeam)
        {
            Team newTeam;
            newTeam.players.push_back(player);
            teams.push_back(newTeam);
        }

        // �� ���� ����� �α�
        printf("[DEBUG] ���� ��⿭ ũ��: %zu\n", playerQueue.size());
        for (const auto& team : teams)
        {
            printf("[DEBUG] �� ũ��: %zu, �÷��̾�: ", team.players.size());
            for (const auto& p : team.players)
            {
                printf("%s ", p.cid.c_str());
            }
            printf("\n");
        }

        // ��� ���� �ϼ��Ǿ����� Ȯ��
        if (teams.size() <= MAX_TEAMS &&
            std::all_of(teams.begin(), teams.end(), [](const Team& team) { return team.isComplete(); }))
        {
            printf("[INFO] ��� ���� �ϼ��Ǿ����ϴ�. ��Ī �Ϸ� ��Ŷ ����.\n");
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
            ZeroMemory(pSocketInfo, sizeof(SOCKETINFO)); // �ʱ�ȭ
            pSocketInfo->socket = player.socket; //�̺κп��� �÷��̾������� Ȱ��
            printf("[DEBUG] Sending to Socket: %lld, Data: MatchComplete\n", (long long)player.socket);
            // ������ ���� ����
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
                printf("[ERROR] ��Ī �Ϸ� ��Ŷ ���� ���� - WSA Error: %d\n", WSAGetLastError());
            }
            else
            {
                printf("[INFO] �޽��� �۽� - Bytes: [%d], HeaderCode: [%d], Msg: [%s]\n",
                    sendBytes,
                    matchCompletePacket.headercode,
                    sendBuffer);
            }

            delete pSocketInfo; // �޸� ����
        }
    }
}
