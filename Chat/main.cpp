#include "stdafx.h"
#include <format>

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

		userConnection();
	}
	void loop() {
		for (int i = 0; i < 16; ++i) {
			Buffer buf(128);
			DWORD len = 0, flag = 0;
			if (WSARecv(clients[i].first, &buf, 1, &len, &flag, &overlap, NULL) == SOCKET_ERROR) {
				if (WSAGetLastError() == WSA_IO_PENDING) {
					WSAWaitForMultipleEvents(1, &evnt, TRUE, WSA_INFINITE, FALSE);
					WSAGetOverlappedResult(clients[i].first, &overlap, &len, TRUE, &flag);
					// 메세지 구분.
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		}
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
	void userConnection() {
		std::thread conn[16];
		std::generate(std::begin(conn), std::end(conn), [this, i=0]() mutable {
			return std::thread([this](int i) {
				clients[i].first = accept(listen_sock, (SOCKADDR*)&clients[i].second, &addrlen);
				if (clients[i].first == INVALID_SOCKET) {
					ShowAPIErrorMessage();
					return;
				}
				std::cout << "[System] Connected : " << clients[i].first << '\n';

				// [방][닉네임] 메세지 형식의 문자열 수신해 로그인이나 채팅 처리
				/*char userName[32] = "", roomName[16] = "";
				while (true) {
					if (recv(clients[i].first, userName, 32, NULL) == SOCKET_ERROR) {
						ShowAPIErrorMessage();
					}
					if (std::find(userList.begin(), userList.end(), userName) == userList.end()) {
						userList.push_back(userName);
						std::cout << "[System] Joined " << userName << '\n';
						std::string msg = "not";
						if (send(clients[i].first, msg.c_str(), msg.length(), NULL) == SOCKET_ERROR) {
							ShowAPIErrorMessage();
						}
					}
					else {
						std::string msg = "exist";
						if (send(clients[i].first, msg.c_str(), msg.length(), NULL) == SOCKET_ERROR) {
							ShowAPIErrorMessage();
						}
						continue;
					}
					break;
				}

				if (recv(clients[i].first, roomName, 16, NULL) == SOCKET_ERROR) {
					ShowAPIErrorMessage();
				}

				if (room.find(roomName) == room.end()) {
					std::cout << "[System] Created room \'" << roomName << "\'\n";
				}
				else {
					std::cout << "[System] " << userName << " entered room \'" << roomName << "\'\n";
				}

				room.insert({ userName, roomName });
				roomList.push_back(roomName);*/
			}, i++);
		});
		for (auto& t : conn) {
			t.detach();
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
		std::string data() {
			return buf;
		}
	private:
		WSABUF wb;
		char* buf;
	};
private:
	WSADATA wsaData;
	SOCKET listen_sock;
	SOCKADDR_IN servAdr;
	int szAdr = sizeof(SOCKADDR_IN);

	std::pair<SOCKET, SOCKADDR_IN> clients[16];
	std::vector<std::string> roomList;
	std::vector<std::string> userList;
	std::mutex mtx;
	int addrlen = sizeof(SOCKADDR_IN);

	std::map<std::string, std::string> room;

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