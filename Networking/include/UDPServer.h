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
	struct Client {
		std::string ip;
		unsigned short port;
	};

	class UdpServer
	{
	public:
		UdpServer();
		UdpServer(const UdpServer&) = delete;
		UdpServer(const UdpServer&&) = delete;
		UdpServer& operator=(const UdpServer&) = delete;
		UdpServer& operator=(const UdpServer&&) = delete;

		~UdpServer();

		int Bind(unsigned short const inPort, unsigned short const outPort, bool const beacon);
		[[nodiscard]] std::string ReceiveData(int const maxSize,Client* outCli) const;
		int SendData(std::string const &data, Client const& client) const;  // NOLINT(modernize-use-nodiscard)
		[[nodiscard]] int BroadcastData(std::string const &data, u_short const port) const;
		[[nodiscard]] int GetInboundPort() const;
		[[nodiscard]] int GetOutboundPort() const;
		[[nodiscard]] std::string GetLocalIp() const;
	protected:
		unsigned short m_inPort;
		unsigned short m_outPort;
		struct addrinfo* m_bind_address;
		SOCKET m_socket_listen;
		SOCKET m_socket_emit;
		SOCKET m_socket_beacon;
		std::ofstream* m_logfile;
		std::string m_local_ip;
	private:
		void log(std::string const &data) const;
	};

}