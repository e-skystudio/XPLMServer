#include "../include/UDPServer.h"

UDPServer::UDPServer() : 
    m_port(0),
    m_hints({0}),
    m_bind_address{0},
    m_socket_listen(0),
    m_socket_emit(0)
{
#ifdef _WIN32
	errno_t error = fopen_s(&m_fout, "XPLMServerNetwork.log", "w+");
#else
	m_fout = fopen("XPLMServerNetwork.log", "w+");
#endif
	log("Loging Started\n");
}

UDPServer::~UDPServer()
{
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
		log("bind() failed on" + std::to_string(port) + "! Error code: " + std::to_string(GETSOCKETERRNO()) + "\n");
		return 0x02;
	}
	log("bind() sucess on " + std::to_string(port) + "!\n");
	return 0x00;
}

std::string UDPServer::ReceiveData(int maxSize,Client* outCli)
{
    if (maxSize < 0)
	{
		log("MaxSize < 0! ERROR\n");
		return std::string();
	}
	struct sockaddr_storage client_address = {0};
	socklen_t client_len = sizeof(client_address);
	char* read = (char *)malloc((size_t)maxSize);
	if (read == nullptr)
		return std::string();
	memset(read, 0x00, (size_t)maxSize);
	struct timeval timeout{0, 10};

	fd_set clients;
	FD_ZERO(&clients);
	FD_SET(m_socket_listen, &clients);

	if(select((int)m_socket_listen + 1, &clients, 0, 0, &timeout) <= 0) return std::string();
	
	int bytes_received = recvfrom(m_socket_listen, read, maxSize, 0,
		(struct sockaddr*)&client_address, &client_len);
	if (bytes_received <= 0)
	{
		log(">>> " + std::to_string(bytes_received) + "bytes received\n");
	}
	char address_buffer[100];
	char service_buffer[100];
	getnameinfo(((struct sockaddr*)&client_address),
		client_len, address_buffer, sizeof(address_buffer),
		service_buffer, sizeof(service_buffer),
		NI_NUMERICHOST | NI_NUMERICSERV);
	outCli->ip = std::string(address_buffer);
	outCli->port = m_port + 1;
	std::string s_out(read);
	s_out.resize(bytes_received);
	char logBuffer[4150];
	#ifdef IBM
	sprintf_s(logBuffer, 4150, "[%s:%s]>>>'%s'(%d byte(s))\n", address_buffer, service_buffer, s_out.c_str(), bytes_received);
	#else
	sprintf(logBuffer, "[%s:%s]>>>'%s'(%d byte(s))\n", address_buffer, service_buffer, s_out.c_str(), bytes_received);
	#endif
	log(std::string(logBuffer));
	return s_out;
}

int UDPServer::SendData(std::string data, Client cli)
{
	struct sockaddr_in send_address;
	memset(&send_address, 0, sizeof(struct sockaddr_in));
	int res = inet_pton(AF_INET, cli.ip.c_str(), &send_address.sin_addr.s_addr);
	if (res < 0)
	{
		log("Error message with InetPton() : " + std::to_string(GETSOCKETERRNO()) + "\n");
	}
	else if (res == 0)
	{
		log("Client address is not valid !\n");
	}
	send_address.sin_family = AF_INET;
	send_address.sin_port = htons(cli.port);
	int bytes = sendto(m_socket_emit, data.c_str(), (int)data.length(), 0, 
		(struct sockaddr*)&send_address, (int)sizeof(struct sockaddr_in));
	char logBuffer[4150];
	#ifdef IBM
	sprintf_s(logBuffer, 4150, "[%s:%d]<<<'%s'(%d byte(s))\n", cli.ip.c_str(), cli.port, data.c_str(), bytes);
	#else
	sprintf(logBuffer, "[%s:%d]<<<'%s'(%d byte(s))\n", cli.ip.c_str(), cli.port, data.c_str(), bytes);
	#endif
	log(std::string(logBuffer));
	if (bytes <= 0)
	{
		log("Error message : " + std::to_string(GETSOCKETERRNO()) + "\n");
	}
	return bytes;
}

void UDPServer::log(std::string data) const
{
	if (m_fout == 0) return;
	fprintf(m_fout, "%s", data.c_str());
	fflush(m_fout);
}
