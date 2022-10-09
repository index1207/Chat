#pragma comment(lib, "ws2_32.lib")

#include <iostream>

#include <WinSock2.h>
#include <WS2tcpip.h>

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
		char userName[32] = "", roomName[16] = "";
		while (true) {
			std::cout << "User : ";
			std::cin >> userName;
			if (send(sock, userName, 32, NULL) == SOCKET_ERROR) {
				show_ws2_err();
			}
			char buf[16];
			if (recv(sock, buf, 16, NULL) == SOCKET_ERROR) {
				show_ws2_err();
			}
			if (strcmp(buf, "exist")) break;
			else {
				std::cout << "이미 존재하는 닉네임입니다.\n";
			}
		}
		std::cout << "Room : ";
		std::cin >> roomName;
		if (send(sock, roomName, 16, NULL) == SOCKET_ERROR) {
			show_ws2_err();
		}
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