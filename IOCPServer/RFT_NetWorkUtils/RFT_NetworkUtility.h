#pragma once
// winsock2 사용을 위해 아래 코멘트 추가
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <windows.h>
#include <iostream>

// Winsock 초기화 및 종료
void InitializeWinSock();
void CleanupWinSock();

// 에러 처리
void err_quit(const char* msg);
void err_display(const char* msg);

// IOCP 생성 및 스레드 관리
void CreateCompletionPort();
