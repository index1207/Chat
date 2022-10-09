#pragma once

#include "stdafx.h"

namespace net {
	void ErrorMessage(std::string text, HWND hWnd = NULL) {
		MessageBox(hWnd, TEXT(text.c_str()), TEXT("[ERROR]"), MB_ICONERROR | MB_OK);
		exit(EXIT_FAILURE);
	}
	void init() {
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
			ErrorMessage("�ʱ�ȭ���� ���߽��ϴ�.");
		}
	}
	void terminate() {
		WSACleanup();
	}
}