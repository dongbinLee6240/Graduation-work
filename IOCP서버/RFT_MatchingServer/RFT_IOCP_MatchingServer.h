#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "RFT_NetworkUtility.h" // 유틸리티 파일을 포함하여 중복 제거
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
    SOCKET			socket;//맞아요 
    char			messageBuffer[BUFSIZE];
    int				recvBytes;
    int				sendBytes;
};


class IOCompletionPort
{
public:
    IOCompletionPort();
    ~IOCompletionPort();

    // 소켓 등록 및 서버 정보 설정
    bool Initialize();
    // 서버 시작
    void StartServer();
    // 작업 스레드 생성
    bool CreateWorkerThread();
    // 작업 스레드
    void WorkerThread();

private:
    SOCKETINFO* pSocketInfo;		// 소켓 정보
    SOCKET			listenSocket;		// 서버 리슨 소켓
    HANDLE			hIOCP;			// IOCP 객체 핸들
    bool			bAccept;			// 요청 동작 플래그
    bool			bWorkerThread;	// 작업 스레드 동작 플래그
    HANDLE* pWorkerHandle;	// 작업 스레드 핸들
};

struct Player 
{
    int playername;
    string cid;
    int skillLevel;
    SOCKET socket;
    
    // 기타 필요한 플레이어 정보

public:

};

struct Team 
{
    std::vector<Player> players;
    bool isComplete() const { return players.size() == 2; }
};