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

#include <stdio.h>
#include <string>
#include <memory.h>

extern "C"{

	struct Client {
		std::string ip;
		unsigned short port;
	};

	class UDPServer
	{
	public:
		UDPServer();
		~UDPServer();
		int Bind(unsigned short port);
		std::string ReceiveData(int maxSize,Client* outCli);
		int SendData(std::string data, Client client);

	protected:
		unsigned short m_port;
		struct addrinfo m_hints;
		struct addrinfo* m_bind_address;
		SOCKET m_socket_listen;
		SOCKET m_socket_emit;
		FILE* m_fout;
	private:
		void log(std::string data) const;
	};

}