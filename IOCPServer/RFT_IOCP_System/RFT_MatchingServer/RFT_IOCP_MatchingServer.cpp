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

// 워커 스레드 진입점 (전역 혹은 static 함수)
unsigned int WINAPI CallWorkerThread(LPVOID p)
{
	IOCompletionPort* pOverlappedEvent = (IOCompletionPort*)p;
	pOverlappedEvent->WorkerThread(); //여기에 소켓 정보를 매개변수로 넘겨야하나
	return 0;
}

IOCompletionPort::IOCompletionPort()
{
	hIOCP = NULL;
	listenSocket = INVALID_SOCKET;
	pWorkerHandle = NULL;
	bWorkerThread = true;
	bAccept = true;
}

IOCompletionPort::~IOCompletionPort()
{
	WSACleanup();
	if (pWorkerHandle) 
	{
		delete[] pWorkerHandle;
		pWorkerHandle = NULL;
	}
}

//FormatMessage를 활용한 err_display함수
void IOCompletionPort::err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void IOCompletionPort::SendPacket(SOCKET targetSocket, Protocol::MessageId msgId, const google::protobuf::Message& message)
{
	// 1. 패킷 조립기(PacketHandler)를 통해 헤더와 데이터를 직렬화
	std::string sendData = PacketHandler::MakeSendBuffer(msgId, message);

	// 2. 이 송신 작업만을 위한 일회용 컨텍스트 박스 동적 생성
	IO_CONTEXT* pSendContext = new IO_CONTEXT();
	pSendContext->io_type = IO_TYPE::IO_SEND;

	// 3. 데이터 복사 및 버퍼 세팅
	// 주의: 실무에서는 sendData가 BUFSIZE를 넘지 않는지 예외 처리를 하지만, 일단 생략합니다.
	memcpy(pSendContext->buffer, sendData.c_str(), sendData.length());
	pSendContext->wsaBuf.buf = pSendContext->buffer;
	pSendContext->wsaBuf.len = static_cast<ULONG>(sendData.length());

	DWORD sendBytes = 0;

	// 4. 운영체제에게 비동기 지시 (Overlapped)
	int nResult = WSASend(targetSocket, &(pSendContext->wsaBuf), 1, &sendBytes, 0, &(pSendContext->overlapped), NULL);

	if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
		printf_s("[ERROR] WSASend 호출 실패 - Socket: %lld\n", (long long)targetSocket);
		delete pSendContext; // 실패 시 메모리 누수 방지를 위해 바로 삭제
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
		err_display("WSAStartup Failed");
		return false;
	}

	// listen소켓 생성
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		err_display("소켓 생성 실패");
		return false;
	}

	// 서버 정보 설정
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVERPORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 소켓 설정
	nResult = ::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	if (nResult == SOCKET_ERROR)
	{
		err_display("bind() 실패");
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	// 수신 대기열 생성
	nResult = listen(listenSocket, SOMAXCONN);
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

    hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    if (!CreateWorkerThread()) return;

	printf_s("[INFO] RFT Matching Server Started on Port %d...\n", SERVERPORT);

    while (bAccept)
    {
        SOCKET clientSocket = WSAAccept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen, NULL, NULL);

        if (clientSocket == INVALID_SOCKET)
        {
            printf_s("[ERROR] Accept Fail - WSA Error: %d\n", WSAGetLastError());
            continue;
        }

        printf_s("[INFO] Client Connect. Socket: %lld\n", (long long)clientSocket);

        // **각 클라이언트마다 독립된 SOCKETINFO 객체 생성**
		SOCKETINFO* pSocketInfo = new SOCKETINFO();
		pSocketInfo->socket = clientSocket;

		//초기화(초기화하지 않은 메모리 주소는 에러: 10014를 반환한다)
		ZeroMemory(&(pSocketInfo->recvContext.overlapped), sizeof(WSAOVERLAPPED));
		pSocketInfo->recvContext.io_type = IO_TYPE::IO_RECV;

		//버퍼를 받을 wsaBuf세팅
		pSocketInfo->recvContext.wsaBuf.buf = pSocketInfo->recvContext.buffer;
		pSocketInfo->recvContext.wsaBuf.len = BUFSIZE;

        // IOCP에 정확히 등록 Completion Key로 pSocketInfo를 넘김
        if (CreateIoCompletionPort((HANDLE)clientSocket, hIOCP, (ULONG_PTR)pSocketInfo, 0) == NULL)
        {
            printf_s("[ERROR] CreateIoCompletionPort fail - Socket: %lld, WSA Error: %d\n",
                     (long long)clientSocket, WSAGetLastError());
            closesocket(clientSocket);
            delete pSocketInfo;
            continue;
        }

        // **첫 번째 수신 작업 설정**
		DWORD recvBytes = 0;
		DWORD flags = 0;

		//recvContext.overlapped 를 넘깁니다
		nResult = WSARecv(
			pSocketInfo->socket,
			&(pSocketInfo->recvContext.wsaBuf),
			1,
			&recvBytes,
			&flags,
			&(pSocketInfo->recvContext.overlapped),
			NULL
		);

		if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
		{
			int errorCode = WSAGetLastError();
			printf_s("[ERROR] WSARecv Failed - Socket: %lld, ErrorCode: %d\n", (long long)clientSocket, errorCode);
			closesocket(clientSocket);
			delete pSocketInfo;
			continue;
		}

    }
}

bool IOCompletionPort::CreateWorkerThread()
{
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int nThreadCnt = sysInfo.dwNumberOfProcessors * 2;

	// thread handler 선언
	pWorkerHandle = new HANDLE[nThreadCnt];
	unsigned int threadId;

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
	printf_s("[INFO] Worker Thread Pool Created (Count: %d)\n", nThreadCnt);
	return true;
}

void IOCompletionPort::WorkerThread()
{
	BOOL bResult;
	DWORD bytesTransferred = 0;
	// 핵심: 키(Session)와 값(IO Context)을 분리해서 받습니다.
	SOCKETINFO* pSocketInfo = NULL; 
	IO_CONTEXT* pContext = NULL;    

	while (bWorkerThread)
	{
		//LPOVERLAPPED 캐스팅을 IO_CONTEXT* 로 받습니다.
		bResult = GetQueuedCompletionStatus(
			hIOCP, 
			&bytesTransferred, 
			(PULONG_PTR)&pSocketInfo, 
			(LPOVERLAPPED*)&pContext, 
			INFINITE);

		// 클라이언트 접속 종료 처리
		if (!bResult || (bytesTransferred == 0 && pContext->io_type == IO_TYPE::IO_RECV)) 
		{
			// 접속 종료 로직
			closesocket(pSocketInfo->socket);
			delete pSocketInfo;
			continue;
		}

		// pSocketInfo가 아니라 pContext의 io_type을 확인합니다
		if (pContext->io_type == IO_TYPE::IO_SEND)
		{
			// 나중에 Send 할 때 new IO_CONTEXT를 만들어서 보낼 것이므로, 완료되면 메모리만 해제합니다.
			// (pSocketInfo를 지우면 안 됩니다! 유저는 아직 접속 중이니까요)
			delete pContext;
			continue;
		}

		// 1. pContext->buffer 에 담긴 '날것의 데이터'를 pSocketInfo의 '링 버퍼'로 밀어넣음
		if (!pSocketInfo->ringBuffer.Enqueue(pContext->buffer, bytesTransferred))
		{
			printf_s("[ERROR] RingBuffer Overflow! Socket: %lld\n", (long long)pSocketInfo->socket);
			closesocket(pSocketInfo->socket);
			delete pSocketInfo;
			continue;
		}

		// 2. 링버퍼에 쌓인 데이터에서 온전한 패킷들을 모두 뽑아냅니다. (뭉쳐서 올 수 있으므로 while문)
		while (true)
		{
			// 1.버퍼에 최소한 Header크기의 4바이트가 모였는가?
			if (pSocketInfo->ringBuffer.GetUsedSize() < HEADER_SIZE) 
			{
				break; // 덜 왔으므로 다음 WSARecv를 기다림
			}

			// 헤더만 살짝 훔쳐보기 (데이터를 빼내지 않음)
			PacketHeader header;
			pSocketInfo->ringBuffer.Peek(reinterpret_cast<char*>(&header), HEADER_SIZE);

			// 2.송장에 적힌 전체 패킷 크기만큼 버퍼에 데이터가 모였는가?
			if (pSocketInfo->ringBuffer.GetUsedSize() < header.packetSize)
			{
				break; // 헤더는 왔지만 Protobuf 내용물이 아직 덜 옴. 대기!
			}

			// ==========================================
			// --- 완전한 패킷 1개 획득! ---
			// ==========================================

			// 3.링버퍼에서 완벽한 패킷 사이즈만큼 빼오기 (Dequeue)
			std::vector<char> fullPacket(header.packetSize);
			pSocketInfo->ringBuffer.Dequeue(fullPacket.data(), header.packetSize);

			// 4.헤더 뒤에 붙어있는 순수 Protobuf 데이터(Payload) 추출
			size_t payloadSize = header.packetSize - HEADER_SIZE;  //패킷사이즈에서 헤더 크기를 뺀 값
			std::string payload(fullPacket.data() + HEADER_SIZE, payloadSize);

			MatchManager& matchManager = MatchManager::GetInstance();

			// 3. 패킷 종류에 따라 Protobuf 객체로 역직렬화 및 비즈니스 로직 처리
			switch (header.messageId)
			{
			case Protocol::REQ_MATCH:
			{
				Protocol::C2S_MatchRequest req;
				if (req.ParseFromString(payload))
				{
					printf_s("[INFO] 매칭 요청 수신! Player ID: %d\n", req.player_id());
					matchManager.addPlayerToQueue(req.player_id(), pSocketInfo->socket, this);
				}
				else 
				{
					printf_s("[ERROR] REQ_MATCH 파싱 실패!\n");
				}
				break;
			}
			default:
				printf_s("[WARNING] 알 수 없는 Message ID: %d\n", header.messageId);
				break;
			}
		}

		// 다음 RECV 작업 등록(초기화)
		ZeroMemory(&(pSocketInfo->recvContext.overlapped), sizeof(WSAOVERLAPPED));
		pSocketInfo->recvContext.io_type = IO_TYPE::IO_RECV; // 혹시 몰라 타입 재명시
		pSocketInfo->recvContext.wsaBuf.buf = pSocketInfo->recvContext.buffer;
		pSocketInfo->recvContext.wsaBuf.len = BUFSIZE;

		DWORD flags = 0;

		int nResult = WSARecv(
			pSocketInfo->socket,
			&(pSocketInfo->recvContext.wsaBuf),
			1,
			&bytesTransferred,
			&flags,
			&(pSocketInfo->recvContext.overlapped),
			NULL
		);

		if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			closesocket(pSocketInfo->socket);
			delete pSocketInfo;
		}
	}
}



