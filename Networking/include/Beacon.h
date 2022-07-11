#pragma once

#ifdef _WIN32
	#include <WinSock2.h>
	#include <WS2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
	#ifndef _WIN32_WINNT
		#define _WIN32_WINNT 0x0600
	#endif
	#ifndef IPV6_V6ONLY
		#define IPV6_V6ONLY 27
	#endif
	#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
	#define CLOSESOCKET(s) closesocket(s)
	#define GETSOCKETERRNO() (WSAGetLastError())
#else
	#include "UDPServer.h"
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <errno.h>
	#include <memory.h>
	#include "Networking.h"

	#define INVALID_SOCKET 0
	#define ISVALIDSOCKET(s) ((s) > 0)
	#define SOCKET int
	#define CLOSESOCKET(s) close(s)
	#define GETSOCKETERRNO() (errno)
#endif

#include <string>
#include <fstream>


std::string GetCurrentDateTime();

extern "C"{
	class Beacon
	{
	public:
		Beacon();
		Beacon(const Beacon&) = delete;
		Beacon(const Beacon&&) = delete;
		Beacon& operator=(const Beacon&) = delete;
		Beacon& operator=(const Beacon&&) = delete;
		~Beacon();

		int Configure(std::string const& targetIp, unsigned short const outPort, bool const broadcast = false);
		int SendData(std::string const &data) const;  // NOLINT(modernize-use-nodiscard)
	protected:
#ifdef IBM
		WSADATA m_wsa = WSAData(0x00);
#endif
		SOCKET m_socket{INVALID_SOCKET};
		std::ofstream* m_logfile;
		sockaddr_in m_si_other;
		int m_slen = sizeof(m_si_other);
	private:
		void log(std::string const &data) const;
	};

}