//핵심 코드
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "MatchManager.h"
#include "RFT_PacketDefinitions.h"
#include "RFT_IOCP_MatchingServer.h"
#include <process.h>
#include <iostream>
#include <Windows.h>
#include <ctime>	
#include <vector>
#include <string>


using namespace std;

void PrintBufferHex(const char* buffer, size_t length) {
	printf("Buffer (Hex): ");
	for (size_t i = 0; i < length; ++i) {
		printf("%02X ", static_cast<unsigned char>(buffer[i]));
	}
	printf("\n");
}

void PacketMaker::serialize(Packet* packet, char* buffer)
{
	int offset = 0;

	// 패킷 길이 추가
	memcpy(buffer + offset, &packet->length, sizeof(packet->length));
	offset += sizeof(packet->length);

	// 헤더 코드 추가
	memcpy(buffer + offset, &packet->headercode, sizeof(packet->headercode));
	offset += sizeof(packet->headercode);

	// 데이터 추가
	memcpy(buffer + offset, packet->data, strlen(packet->data));
	offset += strlen(packet->data);

	// 종료 문자 추가
	memcpy(buffer + offset, &packet->end, sizeof(packet->end));

	buffer[offset] = '\0';
}

Packet PacketMaker::deserialize(const char* buffer)
{
	Packet packet;
	int offset = 0;

	// 헤더 읽기 (1 byte)
	memcpy(&packet.headercode, buffer + offset, sizeof(INT8));
	offset += sizeof(INT8);

	// 데이터 읽기 (가변 길이)
	packet.data = buffer + offset;
	offset += strlen(packet.data); // 문자열의 길이를 기준으로 이동

	// 종료 코드 읽기 (1 byte)
	memcpy(&packet.end, buffer + offset, sizeof(packet.end));

	return packet;
}

unsigned int WINAPI CallWorkerThread(LPVOID p)
{
	IOCompletionPort* pOverlappedEvent = (IOCompletionPort*)p;
	pOverlappedEvent->WorkerThread(); //여기에 소켓 정보를 매개변수로 넘겨야하나
	return 0;
}

IOCompletionPort::IOCompletionPort()
{
	bWorkerThread = true;
	bAccept = true;
}

IOCompletionPort::~IOCompletionPort()
{
	// winsock 의 사용을 끝낸다
	WSACleanup();
	if (pSocketInfo)
	{
		//사용한 객체 삭제
		delete[] pSocketInfo;
		pSocketInfo = NULL;
	}

	if (pWorkerHandle)
	{
		delete[] pWorkerHandle;
		pWorkerHandle = NULL;
	}
}

bool IOCompletionPort::Initialize()
{
	WSADATA wsaData;
	int nResult;
	// winsock 2.2 버전으로 초기화
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (nResult != 0)
	{
		err_display("WSAStartupFailed");
		return false;
	}

	// 소켓 생성
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		err_display("소켓 생성 실패");
		return false;
	}

	// 서버 정보 설정
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVERPORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 소켓 설정
	nResult = bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	if (nResult == SOCKET_ERROR)
	{
		err_display("bind() 실패");
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	// 수신 대기열 생성
	nResult = listen(listenSocket, 5);
	if (nResult == SOCKET_ERROR)
	{
		err_display("listen() 실패");
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	return true;
}

void IOCompletionPort::StartServer()
{
    int nResult;
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(SOCKADDR_IN);
    DWORD recvBytes;
    DWORD flags;
	int cnt = 0;

    hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    if (!CreateWorkerThread()) return;

    printf_s("[INFO] Server Start\n");

    while (bAccept)
    {
        SOCKET clientSocket = WSAAccept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen, NULL, NULL);

        if (clientSocket == INVALID_SOCKET)
        {
            printf_s("[ERROR] Accept Fail - WSA Error: %d\n", WSAGetLastError());
            continue;
        }

        printf_s("1. [INFO] Client Connect. Socket: %lld\n", (long long)clientSocket);

        // **각 클라이언트마다 독립된 SOCKETINFO 객체 생성**
        SOCKETINFO* pSocketInfo = new SOCKETINFO();
        ZeroMemory(pSocketInfo, sizeof(SOCKETINFO));

        pSocketInfo->socket = clientSocket;
        pSocketInfo->dataBuf.len = BUFSIZE;
        pSocketInfo->dataBuf.buf = pSocketInfo->messageBuffer;
		pSocketInfo->io_type = IO_RECV;

		//pSocketInfo이 recv한 횟수 + 소켓의 고유 번호
		//for(int i=0; i<sizeof(pSocketInfo))
		printf("2. recv count: %d, (startserver)pSocketInfo->socket num: %lld\n", cnt, (long long)pSocketInfo->socket);
        // IOCP에 정확히 등록
        if (CreateIoCompletionPort((HANDLE)clientSocket, hIOCP, (ULONG_PTR)pSocketInfo, 0) == NULL)
        {
            printf_s("[ERROR] CreateIoCompletionPort fail - Socket: %lld, WSA Error: %d\n",
                     (long long)clientSocket, WSAGetLastError());
            closesocket(clientSocket);
            delete pSocketInfo;
            continue;
        }

        // **첫 번째 수신 작업 설정**
        recvBytes = 0;
        flags = 0;
		cnt++;

        nResult = WSARecv(
            pSocketInfo->socket,
            &(pSocketInfo->dataBuf),
            1,
            &recvBytes,
            &flags,
            &(pSocketInfo->overlapped),
            NULL
        );

        if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
        {
            printf_s("[ERROR] 초기 WSARecv 실패 - Socket: %lld, WSA Error: %d\n",
                     (long long)clientSocket, WSAGetLastError());
            closesocket(clientSocket);
            delete pSocketInfo;
            continue;
        }

        printf_s("[DEBUG] 3. ClientSocket %lld - 수신 준비 완료\n", (long long)clientSocket);
    }
}

bool IOCompletionPort::CreateWorkerThread()
{
	unsigned int threadId;
	// 시스템 정보 가져옴
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	// 적절한 작업 스레드의 갯수는 (CPU * 2) + 1
	int nThreadCnt = sysInfo.dwNumberOfProcessors * 2;

	// thread handler 선언
	pWorkerHandle = new HANDLE[nThreadCnt];
	// thread 생성
	for (int i = 0; i < nThreadCnt; i++)
	{
		pWorkerHandle[i] = (HANDLE*)_beginthreadex(
			NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &threadId
		);
		if (pWorkerHandle[i] == NULL)
		{
			err_display("[ERROR] Worker Thread 생성 실패\n");
			return false;
		}
		ResumeThread(pWorkerHandle[i]);
	}
	printf_s("[INFO] Thread Pool Complete!\n");
	return true;
}

void IOCompletionPort::WorkerThread()
{
	BOOL bResult;
	DWORD bytesTransferred = 0;
	SOCKETINFO* pSocketInfo = NULL;

	while (bWorkerThread)
	{
		bResult = GetQueuedCompletionStatus(hIOCP,
			&bytesTransferred,
			(PULONG_PTR)&pSocketInfo,
			(LPOVERLAPPED*)&pSocketInfo,
			INFINITE);

		// [변경] 클라이언트 접속 종료 처리 조건 강화
		if (!bResult || (bytesTransferred == 0 && pSocketInfo->io_type == IO_RECV))
		{
			printf_s("[INFO] Client Disconnected - Socket: %lld\n", (long long)pSocketInfo->socket);
			closesocket(pSocketInfo->socket);
			delete pSocketInfo;
			continue;
		}

		// [추가] 송신(SEND)이 완료된 경우의 처리
		if (pSocketInfo->io_type == IO_SEND)
		{
			// 운영체제가 네트워크로 데이터를 다 보냈으므로 메모리 해제
			printf_s("[INFO] Send Completed - Socket: %lld\n", (long long)pSocketInfo->socket);
			delete pSocketInfo;
			continue; // 다시 대기 상태로
		}

		// --- 여기서부터는 수신(RECV) 완료 처리 ---
		pSocketInfo->dataBuf.buf[bytesTransferred] = '\0';

		PacketMaker packetmaker;
		Packet recvpacket = packetmaker.deserialize(pSocketInfo->dataBuf.buf);

		// [변경] GetInstance() 참조 방식으로 호출
		MatchManager& matchManager = MatchManager::GetInstance();

		switch (recvpacket.headercode)
		{
		case REQ_MATCH:
			matchManager.addPlayerToQueue(recvpacket, pSocketInfo->socket);
			matchManager.MatchPlayers();
			break;
		default:
			printf_s("[ERROR] UnKnown HeaderCode: %d\n", recvpacket.headercode);
			break;
		}

		// 다음 RECV 작업 등록
		ZeroMemory(&(pSocketInfo->overlapped), sizeof(OVERLAPPED));
		pSocketInfo->recvBytes = 0;
		pSocketInfo->dataBuf.len = BUFSIZE;
		ZeroMemory(pSocketInfo->messageBuffer, BUFSIZE);
		pSocketInfo->dataBuf.buf = pSocketInfo->messageBuffer;
		pSocketInfo->io_type = IO_RECV; // 다시 RECV 모드로 설정

		DWORD flags = 0;
		int nResult = WSARecv(pSocketInfo->socket,
			&(pSocketInfo->dataBuf),
			1,
			&bytesTransferred,
			&flags,
			&(pSocketInfo->overlapped),
			NULL);

		if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			closesocket(pSocketInfo->socket);
			delete pSocketInfo;
		}
	}
}

