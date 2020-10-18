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

	printf("Hello, this is the UDP client!!\n");

	int af = AF_INET; //IP V4
	int type = SOCK_DGRAM; //UDP
	int protocol = 0;
	int port = 8000;
	int res = 0;
	const char* message = "ping";
	char message_recieved;

	SOCKET s = socket(af, type, protocol);

	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = htons(port);

	sockaddr_in remoteAddr;
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(port);
	const char* remoteAddrStr = "127.0.0.1";
	inet_pton(AF_INET, remoteAddrStr, &remoteAddr.sin_addr);

	while (true) {
		res = sendto(s, message, sizeof(char*), 0, (sockaddr*)&remoteAddr, sizeof(remoteAddr));
		if (res == SOCKET_ERROR) {
			printWSErrorAndExit("No va el enviar mensajes desde cliente!");
		}

		int len = sizeof(bindAddr);
		res = recvfrom(s, &message_recieved, sizeof(char*), 0, (sockaddr*)&bindAddr, &len);
		if (res == SOCKET_ERROR) {
			printWSErrorAndExit("No va el recibir mensajes desde cliente!");
		}
		else {
			printf(&message_recieved);
			Sleep(500);
		}
	}

	//Shutdown socket
	//int shutdown(SOCKET s, int direction);

	//Destroy socket


	system("pause");
	return 0;
}