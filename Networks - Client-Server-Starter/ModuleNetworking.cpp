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

	std::vector<SOCKET> disconnectedSockets;

	for (auto socket : sockets) {
		if (FD_ISSET(socket, &readfdset)) {
			if (isListenSocket(socket)) {
				sockaddr_in clientAddress; //Client address creation
				socklen_t clientAddressSize = sizeof(clientAddress); // Client addrs size

				SOCKET clientSocket = accept(socket, (struct sockaddr*)& clientAddress, &clientAddressSize);

				if (clientSocket < 0)
					reportError("creating connection socket");
				else
				{
					LOG("Client successfully connected");
					addSocket(clientSocket);
					onSocketConnected(clientSocket, clientAddress);
				}					
			}
			else {
				InputMemoryStream packet;
				int bytesRead = recv(socket, packet.GetBufferPtr(), packet.GetCapacity(), 0);
				if (bytesRead > 0) {
					packet.SetSize((uint32)bytesRead);
					onSocketReceivedData(socket, packet);
				}
				else
				{
					if (bytesRead == 0)
						LOG("Client disconnected successfully");
					else
						reportError("receiving message");

					shutdown(socket, 2);
					closesocket(socket);
					disconnectedSockets.push_back(socket);
					onSocketDisconnected(socket);
				}
			}
		}
	}

	for (std::vector<SOCKET>::iterator it = disconnectedSockets.begin(); it != disconnectedSockets.end(); ++it) {
		std::vector<SOCKET>::iterator s_it = std::find(sockets.begin(), sockets.end(), (*it));
		if (s_it != sockets.end()) {
			sockets.erase(s_it);
		}
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

bool ModuleNetworking::sendPacket(const OutputMemoryStream& packet, SOCKET socket)
{
	int result = send(socket, packet.GetBufferPtr(), packet.GetSize(), 0);
	if (result == SOCKET_ERROR)
	{
		reportError("send packet");
		return false;
	}
	return true;
}



void ModuleNetworking::addSocket(SOCKET socket)
{
	sockets.push_back(socket);
}
