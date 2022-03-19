#include "../include/UDPServer.h"

UDPServer::UDPServer() : 
    m_port(0),
    m_hints({0}),
    m_bind_address{0},
    m_socket_listen(0),
    m_socket_emit(0)
{
	m_logfile = new std::ofstream("XPLMServer_Network.log", std::ios::out);
	log("Loging Started");
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
		log("bind() failed on" + std::to_string(port) + "! Error code: " + std::to_string(GETSOCKETERRNO()));
		return 0x02;
	}
	log("bind() sucess on " + std::to_string(port) + "!");
#ifdef IBM
	int optVal = TRUE;
	int iOptLen = sizeof(int);
	int res = setsockopt(m_socket_emit, SOL_SOCKET, SO_BROADCAST, (char*)&optVal, iOptLen);
	log("Setting SOL_SOCKET SO_BROADCAST = 1 returned : '" + std::to_string(res));
	if (res != 0)
	{
		log("Error code is : '" + std::to_string(GETSOCKETERRNO()));
		return 0x03;
	}
#endif
	return 0x00;
}

std::string UDPServer::ReceiveData(int maxSize,Client* outCli)
{
    if (maxSize < 0)
	{
		log("MaxSize < 0! ERROR");
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
		log(">>> " + std::to_string(bytes_received) + "bytes received");
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
	sprintf_s(logBuffer, 4150, "[%s:%s]>>>'%s'(%d byte(s))", address_buffer, service_buffer, s_out.c_str(), bytes_received);
	#else
	sprintf(logBuffer, "[%s:%s]>>>'%s'(%d byte(s))", address_buffer, service_buffer, s_out.c_str(), bytes_received);
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
		log("Error message with InetPton() : " + std::to_string(GETSOCKETERRNO()));
	}
	else if (res == 0)
	{
		log("Client address is not valid !");
	}
	send_address.sin_family = AF_INET;
	send_address.sin_port = htons(cli.port);
	int bytes = sendto(m_socket_emit, data.c_str(), (int)data.length(), 0, 
		(struct sockaddr*)&send_address, (int)sizeof(struct sockaddr_in));
	char logBuffer[4150];
	#ifdef IBM
	sprintf_s(logBuffer, 4150, "[%s:%d]<<<'%s'(%d byte(s))", cli.ip.c_str(), cli.port, data.c_str(), bytes);
	#else
	sprintf(logBuffer, "[%s:%d]<<<'%s'(%d byte(s))", cli.ip.c_str(), cli.port, data.c_str(), bytes);
	#endif
	log(std::string(logBuffer));
	if (bytes <= 0)
	{
		log("Error message : " + std::to_string(GETSOCKETERRNO()));
	}
	return bytes;
}

int UDPServer::BroadcastData(std::string data)
{

	struct sockaddr_in send_address;
	struct sockaddr_in send_address2;
	memset(&send_address, 0, sizeof(struct sockaddr_in));
	memset(&send_address2, 0, sizeof(struct sockaddr_in));
	send_address.sin_family = AF_INET;
	send_address.sin_port = htons(m_port + 1);
	send_address.sin_addr.s_addr = INADDR_BROADCAST;
	send_address2.sin_family = AF_INET;
	send_address2.sin_port = htons(m_port + 1);
	send_address2.sin_addr.s_addr = INADDR_LOOPBACK;
	//int res = inet_pton(AF_INET, cli.ip.c_str(), &send_address.sin_addr.s_addr);
	int bytes = sendto(m_socket_emit, data.c_str(), (int)data.length(), 0,
		(struct sockaddr*)&send_address, (int)sizeof(struct sockaddr_in));
	if (bytes < 0)
	{
		log("There was an error during INET BROADCAST : " + std::to_string(GETSOCKETERRNO()));
	}
	int bytes2 = sendto(m_socket_emit, data.c_str(), (int)data.length(), 0,
		(struct sockaddr*)&send_address2, (int)sizeof(struct sockaddr_in));
	if (bytes2 < 0)
	{
		log("There was an error during LOCALHOST BROADCAST : " + std::to_string(GETSOCKETERRNO()));
}
	char logBuffer[4150];
	#ifdef IBM
		sprintf_s(logBuffer, 4150, "BROADCAST[%d]<<<'%s'(%d byte(s)) & (%d byte(s))",m_port+ 1, data.c_str(), bytes, bytes2);
	#else
		sprintf(logBuffer, "BROADCAST[%d]<<<'%s'(%d byte(s)) & (%d byte(s))", m_port + 1, data.c_str(), bytes, bytes2);
	#endif
	log(std::string(logBuffer));
	return 0;
}

void UDPServer::log(std::string data) const
{
	*m_logfile << GetCurrentDateTime() << "\t" << "UDP" << "\t" << data << "\n";
	m_logfile->flush();
}

std::string GetCurrentDateTime()
{
	struct tm* ltm;
	time_t now = time(0);
#ifdef IBM
	ltm = new struct tm;
	localtime_s(ltm, &now);
#else
	ltm = localtime(&now);
#endif
	char* time = new char[20];
#ifdef IBM
	sprintf_s(time, 20, "%02d/%02d/%04d %02d:%02d:%02d", ltm->tm_mday, ltm->tm_mon, ltm->tm_year,
		ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
#else
	sprintf(time, "%02d/%02d/%04d %02d:%02d:%02d", ltm->tm_mday, ltm->tm_mon, ltm->tm_year,
		ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
#endif
	return std::string((const char*)time);
}
