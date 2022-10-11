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
		char msg[128] = "";
		std::cin >> msg;
		Packet pack(roomName, userName, msg);
		if (WSASend(sock, &pack.buf, 1, &pack.len, pack.flag, &ov, NULL) == SOCKET_ERROR) {
			if (WSAGetLastError() == WSA_IO_PENDING) {
				WSAWaitForMultipleEvents(1, &ev, TRUE, WSA_INFINITE, FALSE);
				WSAGetOverlappedResult(sock, &ov, &pack.len, TRUE, &pack.flag);
			}
			else {
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
	void register_user() {
		std::cout << "User : ";
		std::cin >> userName;
		std::cout << "Room : ";
		std::cin >> roomName;
		Packet pack(roomName, userName, "");
		if (WSASend(sock, &pack.buf, 1, &pack.len, pack.flag, &ov, NULL) == SOCKET_ERROR) {
			if (WSAGetLastError() == WSA_IO_PENDING) {
				WSAWaitForMultipleEvents(1, &ev, TRUE, WSA_INFINITE, FALSE);
				WSAGetOverlappedResult(sock, &ov, &pack.len, TRUE, &pack.flag);
			}
		}
	}
private:
	class Buffer {
	public:
		Buffer(size_t max) : buf(new char[max]) {
			memset(buf, 0, max);
			wb.buf = buf;
			wb.len = max;
		}
		~Buffer() {
			delete[] buf;
		}
		LPWSABUF operator&() {
			return &wb;
		}
		char* operator*() {
			return buf;
		}
		std::string data() {
			return buf;
		}
	private:
		WSABUF wb;
		char* buf;
	};
private:
	std::string userName;
	std::string roomName;

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