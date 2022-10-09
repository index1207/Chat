#include "stdafx.h"

class Chat {
public:
	Chat() {
		if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
			ShowAPIErrorMessage();
		}
	}
	~Chat() {
		WSACleanup();
	}
public:
	void init() {
		listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
		if (listen_sock == INVALID_SOCKET) {
			ShowAPIErrorMessage();
		}

		servAdr.sin_family = AF_INET;
		servAdr.sin_port = htons(1225);
		servAdr.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(listen_sock, (SOCKADDR*)&servAdr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
			ShowAPIErrorMessage();
		}
		if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR) {
			ShowAPIErrorMessage();
		}
		
		std::cout << "[System] Server is running!\n";

		evnt = WSACreateEvent();
		overlap.hEvent = evnt;

		std::thread acceptProc([this]() -> void {
			for (int i = 0; i < 16; ++i) {
				clients[i].first = accept(listen_sock, (SOCKADDR*)&clients[i].second, &addrlen);
				if (clients[i].first == INVALID_SOCKET) {
					ShowAPIErrorMessage();
					return;
				}
				std::cout << "[System] Connected : " << clients[i].first << '\n';
			}
		});
		acceptProc.detach();
	}
	void loop() {

	}
private:
	void ShowAPIErrorMessage() {
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
	}
private:
	WSADATA wsaData;
	SOCKET listen_sock;
	SOCKADDR_IN servAdr;
	int szAdr = sizeof(SOCKADDR_IN);

	std::pair<SOCKET, SOCKADDR_IN> clients[16];
	std::thread acceptProc;
	int addrlen = sizeof(SOCKADDR_IN);

	OVERLAPPED overlap;
	WSAEVENT evnt;
	WSABUF wb;
};

int main() {
	Chat* chat = new Chat();
	chat->init();

	while (true) {
		//chat->loop();
	}
	delete chat;
	return 0;
}