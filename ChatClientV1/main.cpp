#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <format>
#include <thread>
#include <WinSock2.h>
#include <WS2tcpip.h>

class ChatClient {
public:
	ChatClient() : len(0), flag(0) {
		if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
			show_ws2_err();
		}
		wb.buf = buf; wb.len = 1024;
	}
	~ChatClient() {
		WSACleanup();
	}
public:
	void init() {
		memset(&buf, 0, 1024);

		sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
		if (sock == INVALID_SOCKET) {
			show_ws2_err();
		}

		adr.sin_family = AF_INET;
		adr.sin_port = htons(1225);
		inet_pton(AF_INET, "127.0.0.1", &adr.sin_addr);

		ev = WSACreateEvent();
		ov.hEvent = ev;

		connect(sock, (SOCKADDR*)&adr, sizeof(SOCKADDR_IN));
	}
	void loop() {
		memset(&buf, 0, 1024);
		std::cin >> buf;
		if (WSASend(sock, &wb, 1, &len, flag, &ov, NULL) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				show_ws2_err();
			}
		}
	}
private:
	void show_ws2_err() {
		std::wstring message;
		WCHAR buf[512];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			WSAGetLastError(),
			0,
			buf,
			512,
			NULL);
		message = buf;
		MessageBox(NULL, message.c_str(), TEXT("[ERROR]"), MB_ICONERROR | MB_OK);
		exit(EXIT_FAILURE);
	}
private:
	WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN	adr;

	OVERLAPPED ov;
	WSAEVENT ev;

	char buf[1024];
	WSABUF wb;
	DWORD len;
	DWORD flag;
};

int main() {
	ChatClient* client = new ChatClient();
	client->init();

	while (true) {
		client->loop();
	}
	delete client;
}