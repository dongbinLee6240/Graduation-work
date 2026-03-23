#include "RFT_IOCP_MatchingServer.h"
#include "stdafx.h"

int main() 
{
	IOCompletionPort iocp_server;
	if (iocp_server.Initialize())
	{
		iocp_server.StartServer();
	}
	return 0;
}