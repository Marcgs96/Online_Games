#include "Networks.h"
#include "ModuleNetworking.h"
#include <list>


static uint8 NumModulesUsingWinsock = 0;



void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}

void ModuleNetworking::disconnect()
{
	for (SOCKET socket : sockets)
	{
		shutdown(socket, 2);
		closesocket(socket);
	}

	sockets.clear();
}

bool ModuleNetworking::init()
{
	if (NumModulesUsingWinsock == 0)
	{
		NumModulesUsingWinsock++;

		WORD version = MAKEWORD(2, 2);
		WSADATA data;
		if (WSAStartup(version, &data) != 0)
		{
			reportError("ModuleNetworking::init() - WSAStartup");
			return false;
		}
	}

	return true;
}

bool ModuleNetworking::preUpdate()
{
	if (sockets.empty()) return true;

	// NOTE(jesus): You can use this temporary buffer to store data from recv()
	const uint32 incomingDataBufferSize = Kilobytes(1);
	byte incomingDataBuffer[incomingDataBufferSize];

	// TODO(jesus): select those sockets that have a read operation available
	fd_set readfdset;
	FD_ZERO(&readfdset);

	for (auto s : sockets)
	{
		FD_SET(s, &readfdset);
	}

	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	int result = select(0, &readfdset, nullptr, nullptr, &timeout);
	if (result == SOCKET_ERROR) {
		reportError("Error selecting readable sockets");
	}

	// TODO(jesus): for those sockets selected, check wheter or not they are
	// a listen socket or a standard socket and perform the corresponding
	// operation (accept() an incoming connection or recv() incoming data,
	// respectively).
	// On accept() success, communicate the new connected socket to the
	// subclass (use the callback onSocketConnected()), and add the new
	// connected socket to the managed list of sockets.
	// On recv() success, communicate the incoming data received to the
	// subclass (use the callback onSocketReceivedData()).
	// TODO(jesus): handle disconnections. Remember that a socket has been
	// disconnected from its remote end either when recv() returned 0,
	// or when it generated some errors such as ECONNRESET.
	// Communicate detected disconnections to the subclass using the callback
	// onSocketDisconnected().

	std::list<SOCKET> disconnectedSockets;

	for (auto s : sockets) {
		if (FD_ISSET(s, &readfdset)) {
			if (isListenSocket(s)) {
				//Client address creation
				sockaddr_in clientAddress;
				clientAddress.sin_family = AF_INET; // IPv4
				clientAddress.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address
				socklen_t clientAddressSize = sizeof(clientAddress); // Client addrs size

				result = accept(s, (struct sockaddr*)& clientAddress, &clientAddressSize);

				if (result < 0)
					reportError("creating connection socket");
				else
				{
					LOG("Client successfully connected");
					addSocket(result);
					onSocketConnected(result, clientAddress);
				}					
			}
			else {
				result = recv(s, (char*)incomingDataBuffer, incomingDataBufferSize, 0);
				if (result != SOCKET_ERROR) {
					incomingDataBuffer[result] = '\0';
					onSocketReceivedData(s, incomingDataBuffer);
				}
				else
				{
					reportError("receiving message");
					shutdown(s, 2);
					closesocket(s);
					disconnectedSockets.push_back(s);
					onSocketDisconnected(s);
				}
			}
		}
	}

	// TODO(jesus): Finally, remove all disconnected sockets from the list
	// of managed sockets.
	for (std::vector<SOCKET>::iterator it = sockets.begin(); it != sockets.end();) {
		if (std::find(disconnectedSockets.begin(), disconnectedSockets.end(), (*it)) != disconnectedSockets.end()) {
			it = sockets.erase(it);
		}
		else
			++it;
	}



	return true;
}

bool ModuleNetworking::cleanUp()
{
	disconnect();

	NumModulesUsingWinsock--;
	if (NumModulesUsingWinsock == 0)
	{

		if (WSACleanup() != 0)
		{
			reportError("ModuleNetworking::cleanUp() - WSACleanup");
			return false;
		}
	}

	return true;
}

void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}
