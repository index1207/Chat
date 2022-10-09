#pragma once

#include "stdafx.h"
#include "Network.hpp"

namespace net {
	class IPAddress {
	public:
		IPAddress(std::vector<int> address) {
			if (address.size() != 4) {
				ErrorMessage("유효하지 않은 IP주소입니다.");
			}
			
			m_adrStr = std::to_string(address[0]) + ":";
			m_adrStr += std::to_string(address[1]) + ":";
			m_adrStr += std::to_string(address[2]) + ":";
			m_adrStr += std::to_string(address[3]);
			std::cout << m_adrStr;
		}
		IPAddress(IPAddress&& ipadr) noexcept {
			this->m_adrStr = move(ipadr.m_adrStr);
		}
	public:
		static IPAddress Any() {
			return IPAddress::Parse("0.0.0.0");
		}
		static IPAddress LoopBack() {
			return IPAddress::Parse("127.0.0.1");
		}
		static IPAddress Parse(std::string adr) {
			bool isIPV4 = true;
		}
	private:
		std::string m_adrStr;
	};
}