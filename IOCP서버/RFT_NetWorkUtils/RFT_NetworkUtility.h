#pragma once
// winsock2 ����� ���� �Ʒ� �ڸ�Ʈ �߰�
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <windows.h>
#include <iostream>

// Winsock �ʱ�ȭ �� ����
void InitializeWinSock();
void CleanupWinSock();

// ���� ó��
void err_quit(const char* msg);
void err_display(const char* msg);

// IOCP ���� �� ������ ����
void CreateCompletionPort();
