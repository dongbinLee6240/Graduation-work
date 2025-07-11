#include "RFT_NetworkUtility.h"
#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <tchar.h>
using namespace std;

// Winsock 초기화 및 종료
void InitializeWinSock() 
{
    WSADATA wsa;
    int result = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (result != 0) 
    {
        printf("WSAStartup failed: %d\n", result);
        exit(1);
    }
    else 
    {
        printf("WSAStartup 성공\n");
    }
}

void CleanupWinSock() 
{
    WSACleanup();
}

// 에러 처리 함수
void err_quit(const char* msg) 
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
    LocalFree(lpMsgBuf);
    exit(1);
}

void err_display(const char* msg) {
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
    printf("[%s] %s\n", msg, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// 패킷 역직렬화 및 출력
//Packet deserializePacket(const char* buffer) {
//    Packet packet;
//    int offset = 0;
//    memcpy(&packet.length, buffer + offset, sizeof(packet.length));
//    offset += sizeof(packet.length);
//    packet.header = buffer + offset;
//    offset += strlen(packet.header) + 1;
//    packet.data = buffer + offset;
//    offset += strlen(packet.data) + 1;
//    memcpy(&packet.end, buffer + offset, sizeof(packet.end));
//    return packet;
//}
//
//void displayPacket(const Packet& packet) {
//    std::wcout << _T("Length: ") << packet.length << _T(", Header: ") << packet.header
//        << _T(", Data: ") << packet.data << _T(", End: ") << std::hex << packet.end << std::dec << std::endl;
//}

// IOCP 생성
void CreateCompletionPort() 
{
    HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (hcp == NULL)
        err_quit("CreateIoCompletionPort()");
}
