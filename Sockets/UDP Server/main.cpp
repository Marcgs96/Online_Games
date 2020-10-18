#include <stdlib.h>
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>

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
	//system("pause");
	//exit(-1);
}

int main() {
	WSADATA wsa_data;

	int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);

	if (result != NO_ERROR) {
		//handle error
		return false;
	}
	printf("Hello, this is the UDP server!!\n");

	int af = AF_INET; //IP V4
	int type = SOCK_DGRAM; //UDP
	int protocol = 0;
	int port = 8000;
	int res = 0;
	SOCKET s = socket(af, type, protocol);
	char message_recieved;
	const char* message = "pong";

	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET; // IPv4
	bindAddr.sin_port = htons(port); // Port
	bindAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address

	res = bind(s, (const struct sockaddr*)&bindAddr, sizeof(bindAddr));

	if (res == SOCKET_ERROR) {
		printWSErrorAndExit("No se ha bindeado el socket!");
	}

	while (true) {
		int len = sizeof(bindAddr);
		res = recvfrom(s, &message_recieved, sizeof(char*), 0, (sockaddr*)&bindAddr, &len);
		if (res == SOCKET_ERROR) {
			printWSErrorAndExit("No va el recibir mensajes!");
		}
		else {
			printf(&message_recieved);
		}
		Sleep(500);
		res = sendto(s, message, sizeof(char*), 0, (sockaddr*)&bindAddr, sizeof(bindAddr));
		if (res == SOCKET_ERROR) {
			printWSErrorAndExit("No va el recibir mensajes!");
		}
	}
	//Shutdown socket
	//int shutdown(SOCKET s, int direction);

	//Destroy socket
	//int closesocket(SOCKET s);

	system("pause");
	return 0;
}