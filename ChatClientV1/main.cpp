#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <format>

#include <WinSock2.h>
#include <WS2tcpip.h>

class Packet {
public:
	Packet(std::string room, std::string id, std::string message)
		: len(0), flag(0)
	{
		buf.buf = cbuf;
		buf.len = 1024;
		auto str = std::format("[{}][{}] {}", room, id, message);
		memcpy(&buf, str.c_str(), str.length());
	}
	WSABUF buf;
	DWORD len;
	DWORD flag;
private:
	char cbuf[1024];
};

namespace std {
	std::string to_string(Packet& pack) {
		return pack.buf.buf;
	}
}
class ChatClient {
public:
	ChatClient() {
		if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
			show_ws2_err();
		}
	}
	~ChatClient() {
		WSACleanup();
	}
public:
	void init() {
		sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
		if (sock == INVALID_SOCKET) {
			show_ws2_err();
		}

		adr.sin_family = AF_INET;
		adr.sin_port = htons(1225);
		inet_pton(AF_INET, "127.0.0.1", &adr.sin_addr);

		connect(sock, (SOCKADDR*)&adr, sizeof(SOCKADDR_IN));

		ev = WSACreateEvent();
		ov.hEvent = ev;

		register_user();
	}
	void loop() {
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
	void register_user() {
	}
private:
	WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN	adr;

	OVERLAPPED ov;
	WSAEVENT ev;
};

int main() {
	ChatClient* client = new ChatClient();
	client->init();

	while (true) {
		client->loop();
	}
	delete client;
}