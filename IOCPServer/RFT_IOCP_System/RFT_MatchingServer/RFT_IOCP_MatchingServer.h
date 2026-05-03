#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "RFT_NetworkUtility.h" // 유틸리티 파일을 포함하여 중복 제거
#include "RFT_PacketDefinitions.h"
#include "RingBuffer.h"
#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <iostream>
#include <vector>

//RFT_IOCP_MatchingServer.h(헤더파일)
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

enum class IO_TYPE 
{ IO_RECV, 
  IO_SEND 
};

struct IO_CONTEXT
{
    WSAOVERLAPPED   overlapped;
    char            buffer[BUFSIZE];
    IO_TYPE         io_type;
    WSABUF          wsaBuf; // 이 작업에 사용될 버퍼 정보
};

void PrintBufferHex(const char* buffer, size_t length);

struct SOCKETINFO
{
    SOCKET          socket;
    RingBuffer      ringBuffer;

    // 핵심: 수신용 행동 상자와 송신용 행동 상자를 '따로' 둡니다!
    IO_CONTEXT      recvContext;
    IO_CONTEXT      sendContext;

    // 생성자에서 초기화
    SOCKETINFO() 
    {
        ZeroMemory(&recvContext.overlapped, sizeof(WSAOVERLAPPED));
        recvContext.io_type = IO_TYPE::IO_RECV;

        ZeroMemory(&sendContext.overlapped, sizeof(WSAOVERLAPPED));
        sendContext.io_type = IO_TYPE::IO_SEND;
    }
};


class IOCompletionPort
{
public:
    IOCompletionPort();
    ~IOCompletionPort();

    bool Initialize();
    void StartServer();
    bool CreateWorkerThread();
    void WorkerThread();
    void SendPacket(SOCKET targetSocket, 
        Protocol::MessageId msgId, 
        const google::protobuf::Message& message);


private:
    void err_display(const char* msg); // 에러 출력용 헬퍼 함수

    HANDLE hIOCP;
    SOCKET listenSocket;
    HANDLE* pWorkerHandle;
    bool bWorkerThread;
    bool bAccept;
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