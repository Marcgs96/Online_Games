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
	WSADATA wsa_data;

	int res = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	if (res != NO_ERROR) {
		printWSErrorAndExit("Error while initializing Sockets API from Server");
		//handle error
		return false;
	}
	printf("Hello, this is the TCP server!!\n");

	const char* message = "pong";
	char message_received[MAX_BUFFER_SIZE];

	//Socket creation
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

	//Remote Address creation
	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET; // IPv4
	bindAddr.sin_port = htons(PORT); // Port
	bindAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address

	//Bind remote address
	res = bind(s, (const struct sockaddr*) & bindAddr, sizeof(bindAddr));
	if (res == SOCKET_ERROR) {
		printWSErrorAndExit("No se ha bindeado el socket!");
	}

	listen(s, 1);

	int len = sizeof(bindAddr);
	SOCKET s2 = accept(s, (struct sockaddr*) &bindAddr, &len);

	while(true)
	{
		res = recv(s2, message_received, sizeof(message), 0);
		if (res == SOCKET_ERROR) {
			printWSErrorAndExit("Server error receiving message");
		}
		else if (res == 0)
		{
			break;
		}
		else {
			message_received[res] = '\0'; //add end of string
			printf("%s\n", message_received);
			Sleep(500);
		}

		res = send(s2, message, sizeof(message), 0);
		if (res == SOCKET_ERROR) {
			printWSErrorAndExit("Server error sending message");
		}
	}

	//Shutdown socket
	//int shutdown(SOCKET s, int direction);

	//Destroy socket
	closesocket(s);
	closesocket(s2);

	//CleanUp Scokets API
	res = WSACleanup();
	if (res != NO_ERROR) {
		printWSErrorAndExit("Error while closing Sockets API from Server");
		//handle error
		return false;
	}

	system("pause");
	return 0;
}