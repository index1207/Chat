#include "stdafx.h"
#include <format>

class Packet {
public:
private:
};

class Buffer {
public:
	Buffer() {
		wb.buf = buf;
		wb.len = 1024;
	}
	DWORD len = 0;
	DWORD flag = 0;
	WSABUF* operator&() {
		return &wb;
	}
	std::string Get() const {
		return buf;
	}
private:
	char buf[1024];
	WSABUF wb;
};

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
		
		AcceptProcess();
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
		exit(EXIT_FAILURE);
	}
	void AcceptProcess() {
		std::thread acc[16];
		std::generate(std::begin(acc), std::end(acc), [this, i=0]() mutable {
			return std::thread([this](int i) {
				clients[i].first = accept(listen_sock, (SOCKADDR*)&clients[i].second, &addrlen);
				if (clients[i].first == INVALID_SOCKET) {
					return;
				}
				std::cout << "[System] Connected " << clients[i].first << '\n';

				std::thread{[&]() {
					while (true) {
						if (WSARecv(clients[i].first, &cb[i], 1, &cb[i].len, &cb[i].flag, &overlap, NULL) == SOCKET_ERROR) {
							if (WSAGetLastError() == WSA_IO_PENDING) {
								WSAWaitForMultipleEvents(1, &evnt, TRUE, WSA_INFINITE, FALSE);
								WSAGetOverlappedResult(clients[i].first, &overlap, &cb[i].len, TRUE, &cb[i].flag);

								std::cout << std::format("[Client({})] {}\n", i, cb[i].Get());
							}
						}
					}
				}}.join();
			}, i++);
		});
		for (auto& thr : acc) {
			thr.detach();
		}
	}
private:
	WSADATA wsaData;
	SOCKET listen_sock;
	SOCKADDR_IN servAdr;
	int szAdr = sizeof(SOCKADDR_IN);

	std::pair<SOCKET, SOCKADDR_IN> clients[16];
	Buffer cb[16];
	int addrlen = sizeof(SOCKADDR_IN);

	std::thread cnn[16];

	OVERLAPPED overlap;
	WSAEVENT evnt;
};

int main() {
	Chat* chat = new Chat();
	chat->init();

	while (true) {
		chat->loop();
	}
	delete chat;
	return 0;
}