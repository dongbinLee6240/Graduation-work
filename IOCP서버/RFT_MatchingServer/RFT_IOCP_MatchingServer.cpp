//�ٽ� �ڵ�
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "MatchManager.h"
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

	// ��Ŷ ���� �߰�
	memcpy(buffer + offset, &packet->length, sizeof(packet->length));
	offset += sizeof(packet->length);

	// ��� �ڵ� �߰�
	memcpy(buffer + offset, &packet->headercode, sizeof(packet->headercode));
	offset += sizeof(packet->headercode);

	// ������ �߰�
	memcpy(buffer + offset, packet->data, strlen(packet->data));
	offset += strlen(packet->data);

	// ���� ���� �߰�
	memcpy(buffer + offset, &packet->end, sizeof(packet->end));

	buffer[offset] = '\0';
}

Packet PacketMaker::deserialize(const char* buffer)
{
	Packet packet;
	int offset = 0;

	// ��� �б� (1 byte)
	memcpy(&packet.headercode, buffer + offset, sizeof(INT8));
	offset += sizeof(INT8);

	// ������ �б� (���� ����)
	packet.data = buffer + offset;
	offset += strlen(packet.data); // ���ڿ��� ���̸� �������� �̵�

	// ���� �ڵ� �б� (1 byte)
	memcpy(&packet.end, buffer + offset, sizeof(packet.end));

	return packet;
}

unsigned int WINAPI CallWorkerThread(LPVOID p)
{
	IOCompletionPort* pOverlappedEvent = (IOCompletionPort*)p;
	pOverlappedEvent->WorkerThread(); //���⿡ ���� ������ �Ű������� �Ѱܾ��ϳ�
	return 0;
}

IOCompletionPort::IOCompletionPort()
{
	bWorkerThread = true;
	bAccept = true;
}

IOCompletionPort::~IOCompletionPort()
{
	// winsock �� ����� ������
	WSACleanup();
	if (pSocketInfo)
	{
		//����� ��ü ����
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
	// winsock 2.2 �������� �ʱ�ȭ
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (nResult != 0)
	{
		err_display("WSAStartupFailed");
		return false;
	}

	// ���� ����
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		err_display("���� ���� ����");
		return false;
	}

	// ���� ���� ����
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(SERVERPORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// ���� ����
	nResult = bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	if (nResult == SOCKET_ERROR)
	{
		err_display("bind() ����");
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	// ���� ��⿭ ����
	nResult = listen(listenSocket, 5);
	if (nResult == SOCKET_ERROR)
	{
		err_display("listen() ����");
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

        // **�� Ŭ���̾�Ʈ���� ������ SOCKETINFO ��ü ����**
        SOCKETINFO* pSocketInfo = new SOCKETINFO();
        ZeroMemory(pSocketInfo, sizeof(SOCKETINFO));

        pSocketInfo->socket = clientSocket;
        pSocketInfo->recvBytes = 0;
        pSocketInfo->sendBytes = 0;
        pSocketInfo->dataBuf.len = BUFSIZE;
        pSocketInfo->dataBuf.buf = pSocketInfo->messageBuffer;
		//for(int i=0; i<sizeof(pSocketInfo))
		printf("2. recv count: %d, (startserver)pSocketInfo->socket num: %lld\n", cnt, (long long)pSocketInfo->socket);
        // IOCP�� ��Ȯ�� ���
        if (CreateIoCompletionPort((HANDLE)clientSocket, hIOCP, (ULONG_PTR)pSocketInfo, 0) == NULL)
        {
            printf_s("[ERROR] CreateIoCompletionPort fail - Socket: %lld, WSA Error: %d\n",
                     (long long)clientSocket, WSAGetLastError());
            closesocket(clientSocket);
            delete pSocketInfo;
            continue;
        }

        // **ù ��° ���� �۾� ����**
        recvBytes = 0;
        flags = 0;
		cnt++;
		/*printf("recv Ƚ��: %d, pSocketInfo->socket����: %lld\n", cnt, (long long)pSocketInfo->socket);*/
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
            printf_s("[ERROR] �ʱ� WSARecv ���� - Socket: %lld, WSA Error: %d\n",
                     (long long)clientSocket, WSAGetLastError());
            closesocket(clientSocket);
            delete pSocketInfo;
            continue;
        }

        printf_s("[DEBUG] 3. ClientSocket %lld - ���� �غ� �Ϸ�\n", (long long)clientSocket);
    }
}

bool IOCompletionPort::CreateWorkerThread()
{
	unsigned int threadId;
	// �ý��� ���� ������
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	// ������ �۾� �������� ������ (CPU * 2) + 1
	int nThreadCnt = sysInfo.dwNumberOfProcessors * 2;

	// thread handler ����
	pWorkerHandle = new HANDLE[nThreadCnt];
	// thread ����
	for (int i = 0; i < nThreadCnt; i++)
	{
		pWorkerHandle[i] = (HANDLE*)_beginthreadex(
			NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &threadId
		);
		if (pWorkerHandle[i] == NULL)
		{
			err_display("[ERROR] Worker Thread ���� ����\n");
			return false;
		}
		ResumeThread(pWorkerHandle[i]);
	}
	printf_s("[INFO] Thread Pool Complete!\n");
	return true;
}

void IOCompletionPort::WorkerThread()
{
	//workerthread ��� ���� Getqueued completeionstatus�� ���ݾƿ� ���� 
	//IOCP��Ʈ���� �۾��� �Ϸ�ɶ����� ����ϴ°ſ��� . �۾��� �ٵǸ� ��Ⱑ Ǯ���� �Ʒ� �۾�
	//
	BOOL bResult;
	DWORD recvBytes = 0;
	DWORD flags = 0;
	SOCKETINFO* pSocketInfo; //�̰� ����ü���� �־�� ���� �׷��� pSocket

	while (bWorkerThread) //while��
	{
		//���� hIOCP ���� �ɰ� �����ؿ�
		// IOCP ť���� �Ϸ�� �۾� �������� ���� ���Ѱ� �꿡�� �� ������ �̺κп��� 
		//Ŭ��1 ������ ��� ����ؼ� ����� �������ƿ� ���⼭ ���� ������ addplayer����ΰ���
		bResult = GetQueuedCompletionStatus(hIOCP, //�� ����ϴٰ� Ŭ�� �����ϰ� ���� �ް� ��Ŷ �ް� �ϸ� �Ʒ��� �ϴ°ſ���
			&recvBytes,
			(PULONG_PTR)&pSocketInfo, //���⼭ SocketInfo ����ؿ�
			(LPOVERLAPPED*)&pSocketInfo,
			INFINITE);
		printf("4. After GQCS pSocketInfo->socket Num: %lld\n", (long long)pSocketInfo->socket);
		if (!bResult || recvBytes == 0) // Ŭ���̾�Ʈ ���� ���� ó��
		{
			printf_s("[INFO] Client Connect Close - Socket: %lld\n", (long long)pSocketInfo->socket);
			closesocket(pSocketInfo->socket);
			delete pSocketInfo;
			continue;
		}
		// ���ŵ� ������ ó��
		pSocketInfo->dataBuf.buf[recvBytes] = '\0'; // ���ڿ� �� ó��
		printf_s("5. [INFO] Message Recv - Socket: %lld, Bytes: [%d], Msg: [%s]\n",
			(long long)pSocketInfo->socket, recvBytes, pSocketInfo->dataBuf.buf);
		//5���� ���� �޼��� ���� �ι��� �־ ����ϰ� �ߴµ� �̰Ŵ� ��� ������ �� �ǹ� �����
		// ��Ŷ ó�� ���� ����
		PacketMaker packetmaker;
		Packet recvpacket = packetmaker.deserialize(pSocketInfo->dataBuf.buf);
		MatchManager* matchManager = MatchManager::GetInstance();

		switch (recvpacket.headercode)
		{
		case REQ_MATCH: //��Ŷ ����� ���� �Լ� �ϰ� �س���� IOCP�κп� ������ �ִ°ǰ�
			printf_s(" 6. [DEBUG] Call addPlayerToQueue - Socket: %lld\n", (long long)pSocketInfo->socket);
			matchManager->addPlayerToQueue(recvpacket, pSocketInfo->socket);
			matchManager->MatchPlayers();
			break;

		default:
			printf_s("[ERROR] UnKnown HeaderCode: %d\n", recvpacket.headercode);
			break;
		}

		// **���� ���� �۾� ���** (�ٽ� �κ�)
		ZeroMemory(&(pSocketInfo->overlapped), sizeof(OVERLAPPED));
		pSocketInfo->recvBytes = 0;
		pSocketInfo->dataBuf.len = BUFSIZE;
		ZeroMemory(pSocketInfo->messageBuffer, BUFSIZE);
		pSocketInfo->dataBuf.buf = pSocketInfo->messageBuffer;

		int nResult = WSARecv(pSocketInfo->socket,
			&(pSocketInfo->dataBuf),
			1,
			&recvBytes,
			&flags,
			&(pSocketInfo->overlapped),
			NULL);

		if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			printf_s("[ERROR] WSARecv ���� - Socket: %lld, WSA Error: %d\n",
				(long long)pSocketInfo->socket, WSAGetLastError());
			closesocket(pSocketInfo->socket);
			delete pSocketInfo;
		}
	}
}

