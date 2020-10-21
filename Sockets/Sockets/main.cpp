#include <stdlib.h>
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>

#define MAX_BUFFER_SIZE 1024
#define PORT 8000

void printWSErrorAndExit(const char* msg)
{
	wchar_t* s = NULL;
	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s,
		0, NULL);
	fprintf(stderr, "%s: %S\n", msg, s);
	LocalFree(s);
	system("pause");
	exit(-1);
}

int main() {
	//Initialize Socket API
	WSADATA wsa_data;

	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	if (result != NO_ERROR) {
		printWSErrorAndExit("Error while initializing Sockets API from Client");
		//handle error
		return false;
	}

	printf("Hello, this is the UDP client!!\n");

	const char* message = "ping";
	char message_received[MAX_BUFFER_SIZE];

	//UDP Socket Creation AF_INET for IPv4 and SOCK_DGRAM for UDP
	SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);

	//Server address creation
	sockaddr_in remoteAddr;
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(PORT);
	const char* remoteAddrStr = "127.0.0.1";
	inet_pton(AF_INET, remoteAddrStr, &remoteAddr.sin_addr);

	for (int i = 0; i < 5; ++i)
	{
		int len = sizeof(remoteAddr);

		result = sendto(s, message, sizeof(message), 0, (sockaddr*)&remoteAddr, len);
		if (result == SOCKET_ERROR) {
			printWSErrorAndExit("Client error sending message");
		}

		result = recvfrom(s, message_received, sizeof(message), 0, (sockaddr*)&remoteAddr, &len);
		if (result == SOCKET_ERROR) {
			printWSErrorAndExit("Client error receiving message");
		}
		else {
			message_received[result] = '\0'; //add end of string
			printf("%s\n", message_received);
		}
	}

	//Shutdown socket
	//int shutdown(SOCKET s, int direction);

	//Destroy socket
	closesocket(s);
	//CleanUp Scokets API
	result = WSACleanup();
	if (result != NO_ERROR) {
		printWSErrorAndExit("Error while closing Sockets API from Client");
		//handle error
		return false;
	}

	system("pause");
	return 0;
}