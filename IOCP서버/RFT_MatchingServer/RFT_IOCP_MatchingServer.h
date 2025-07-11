#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "RFT_NetworkUtility.h" // ��ƿ��Ƽ ������ �����Ͽ� �ߺ� ����
#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <iostream>
#include <vector>

using namespace std;

#define SERVERPORT 9000
#define BUFSIZE    512
#define SERVERIP "127.0.0.1"

enum HeaderType : INT8
{
    REQ_MATCH = 0,
    MATCHING = 1,
    MATCH_COMPLETE = 2
};

void PrintBufferHex(const char* buffer, size_t length);
class Packet
{
public:
    short length;
    INT8 headercode;
    const char* data;
    unsigned short end;

    Packet() : length(0), headercode(0), data(nullptr), end(0xFFFF) {}
    ~Packet() {}

    void EnterMatch(Packet* packet, char* buffer);

    void MatchCompletePacket(Packet* packet,char*buffer);
};

class PacketMaker
{
public:
    void serialize(Packet* packet, char* buffer);

    Packet deserialize(const char* buffer);
};

struct SOCKETINFO
{
    WSAOVERLAPPED	overlapped;
    WSABUF			dataBuf;
    SOCKET			socket;//�¾ƿ� 
    char			messageBuffer[BUFSIZE];
    int				recvBytes;
    int				sendBytes;
};


class IOCompletionPort
{
public:
    IOCompletionPort();
    ~IOCompletionPort();

    // ���� ��� �� ���� ���� ����
    bool Initialize();
    // ���� ����
    void StartServer();
    // �۾� ������ ����
    bool CreateWorkerThread();
    // �۾� ������
    void WorkerThread();

private:
    SOCKETINFO* pSocketInfo;		// ���� ����
    SOCKET			listenSocket;		// ���� ���� ����
    HANDLE			hIOCP;			// IOCP ��ü �ڵ�
    bool			bAccept;			// ��û ���� �÷���
    bool			bWorkerThread;	// �۾� ������ ���� �÷���
    HANDLE* pWorkerHandle;	// �۾� ������ �ڵ�
};

struct Player 
{
    int playername;
    string cid;
    int skillLevel;
    SOCKET socket;
    
    // ��Ÿ �ʿ��� �÷��̾� ����

public:

};

struct Team 
{
    std::vector<Player> players;
    bool isComplete() const { return players.size() == 2; }
};