#include "../include/UDPServer.h"

UDPServer::UDPServer() :
	m_hints{ 0 },
	m_wsa{ 0 },
	m_bind_address(nullptr),
	m_socket_listen(INVALID_SOCKET),
	m_socket_emit(INVALID_SOCKET),
	m_port{ 0 }
{
	fopen_s(&m_fout, "ServerLog.txt", "w+");
	if (m_fout == 0)
		return;
#ifdef _WIN32
	m_wsa;
	if (WSAStartup(MAKEWORD(2, 2), &m_wsa))
	{
		fprintf(m_fout, "Failed to initialize WSA.\n");
		fflush(m_fout);
		return;
	}
#endif
}

UDPServer::~UDPServer()
{
	closesocket(m_socket_listen);
	freeaddrinfo(m_bind_address);
	#ifdef _WIN32
		WSACleanup();
	#endif
}

int UDPServer::Bind(unsigned short port)
{
	m_port = port;
	memset(&m_hints, 0, sizeof(m_hints));
	m_hints.ai_family = AF_INET;
	m_hints.ai_socktype = SOCK_DGRAM;
	m_hints.ai_flags = AI_PASSIVE;
	

	int option = 0;
	getaddrinfo(0, std::to_string(port).c_str(), &m_hints, &m_bind_address);
	m_socket_listen = socket(m_bind_address->ai_family, m_bind_address->ai_socktype, m_bind_address->ai_protocol);
	m_socket_emit = socket(m_bind_address->ai_family, m_bind_address->ai_socktype, m_bind_address->ai_protocol);
	if (bind(m_socket_listen, m_bind_address->ai_addr, (int)m_bind_address->ai_addrlen))
	{
		fprintf(m_fout, "bind() failed on %d! Error code: %d\n", port, GETSOCKETERRNO());
		fflush(m_fout);
		return 0x02;
	}
	return 0x00;
}

std::string UDPServer::ReceiveData(int maxSize, Client* outCli)
{
	if (maxSize < 0)
		return std::string();
	struct sockaddr_storage client_address;
	socklen_t client_len = sizeof(client_address);
	char* read = (char *)malloc((size_t)maxSize);
	if (read == nullptr)
		return std::string();
	memset(read, 0x00, (size_t)maxSize);
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 5000;

	fd_set clients;
	FD_ZERO(&clients);
	FD_SET(m_socket_listen, &clients);

	select((int)m_socket_listen, &clients, 0, 0, &timeout);

	if (FD_ISSET(m_socket_listen, &clients))
	{
		int bytes_received = recvfrom(m_socket_listen, read, maxSize, 0,
			(struct sockaddr*)&client_address, &client_len);
		if (bytes_received <= 0)
		{
			fprintf(m_fout, ">>> %d bytes received\n", bytes_received);
			fflush(m_fout);
		}
		char address_buffer[100];
		char service_buffer[100];
		getnameinfo(((struct sockaddr*)&client_address),
			client_len, address_buffer, sizeof(address_buffer),
			service_buffer, sizeof(service_buffer),
			NI_NUMERICHOST | NI_NUMERICSERV);
		outCli->ip = std::string(address_buffer);
		outCli->port = m_port + 1;
		fprintf(m_fout, "[%s:%s]>>>%s (%d byte(s))\n", address_buffer, service_buffer, read, bytes_received);
		fflush(m_fout);
		std::string s_out(read);
		s_out.resize(bytes_received);
		return s_out;
	}
	return std::string();
}

int UDPServer::SendData(std::string data, Client cli)
{
	struct sockaddr_in send_address;
	memset(&send_address, 0, sizeof(struct sockaddr_in));
	int res = inet_pton(AF_INET, cli.ip.c_str(), &send_address.sin_addr.s_addr);
	if (res < 0)
	{
		fprintf(m_fout, "Error message with InetPton() : %d\n", GETSOCKETERRNO());
		fflush(m_fout);
	}
	else if (res == 0)
	{
		fprintf(m_fout, "Client address is not valid !\n");
		fflush(m_fout);
	}
	send_address.sin_family = AF_INET;
	send_address.sin_port = htons(cli.port);
	//send_address.sin_addr.s_addr = inet_addr(cli.ip.c_str());
	int bytes = sendto(m_socket_emit, data.c_str(), (int)data.length(), 0, 
		(struct sockaddr*)&send_address, (int)sizeof(struct sockaddr_in));
	fprintf(m_fout, "[%s:%d]<<<%s (%d byte(s))\n", cli.ip.c_str(), cli.port, data.c_str(), bytes);
	fflush(m_fout);
	if (bytes <= 0)
	{
		fprintf(m_fout, "Error message : %d\n", GETSOCKETERRNO());
		fflush(m_fout);
	}
	return bytes;
}
