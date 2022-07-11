#include "../include/UDPServer.h"

UdpServer::UdpServer() :
	m_inPort(0),
	m_outPort(0),
    m_bind_address{nullptr},
    m_socket_listen(INVALID_SOCKET),
    m_socket_emit(INVALID_SOCKET),
	m_socket_beacon(INVALID_SOCKET),
	m_local_ip("")

{


	m_logfile = new std::ofstream("XPLMServer_Network.log", std::ios::out);
	log("Loging Started");
}



UdpServer::~UdpServer() = default;


int UdpServer::Bind(unsigned short const inPort, unsigned short const outPort)
{
#ifdef IBM
	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
		return 0x01;
#endif
    m_inPort = inPort;
	m_outPort = outPort;
    addrinfo hints = {};
    hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	int _ = getaddrinfo(nullptr, std::to_string(m_inPort).c_str(), &hints, &m_bind_address);
    m_socket_listen = socket(m_bind_address->ai_family, m_bind_address->ai_socktype, m_bind_address->ai_protocol);
	m_socket_emit = socket(m_bind_address->ai_family, m_bind_address->ai_socktype, m_bind_address->ai_protocol);
	if (bind(m_socket_listen, m_bind_address->ai_addr, static_cast<int>(m_bind_address->ai_addrlen)))
	{
		log("bind() failed on" + std::to_string(m_inPort) + "! Error code: " + std::to_string(GETSOCKETERRNO()));
		return 0x02;
	}
	log("bind() success on " + std::to_string(m_inPort) + "!");
	return 0x00;
}

std::string UdpServer::ReceiveData(int const maxSize,Client* outCli) const
{
    if (maxSize < 0)
	{
		log("MaxSize < 0! ERROR");
		return {};
	}
    sockaddr_storage client_address;  // NOLINT(cppcoreguidelines-pro-type-member-init)
	socklen_t client_len = sizeof(client_address);
    const auto read = static_cast<char*>(malloc(static_cast<size_t>(maxSize)));
	if (read == nullptr)
		return {};
	memset(read, 0x00, static_cast<size_t>(maxSize));
	#ifdef IBM
    	constexpr timeval timeout{0, 10};
	#else
		timeval timeout{0, 10};
	#endif

	fd_set clients;
	FD_ZERO(&clients);
	FD_SET(m_socket_listen, &clients);

	if(select(static_cast<int>(m_socket_listen) + 1, &clients, 0, 0, &timeout) <= 0) return {};
	
	int bytes_received = recvfrom(m_socket_listen, read, maxSize, 0,
		reinterpret_cast<sockaddr*>(&client_address), &client_len);
	if (bytes_received <= 0)
	{
		log(">>> " + std::to_string(bytes_received) + "bytes received");
	}
	char address_buffer[100];
	char service_buffer[100];
	getnameinfo(reinterpret_cast<sockaddr*>(&client_address),
		client_len, address_buffer, sizeof(address_buffer),
		service_buffer, sizeof(service_buffer),
		NI_NUMERICHOST | NI_NUMERICSERV);
	outCli->Ip = std::string(address_buffer);
	outCli->Port = m_outPort;
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

int UdpServer::SendData(std::string const &data, Client const& client) const
{
	sockaddr_in send_address = {};
	int res = inet_pton(AF_INET, client.Ip.c_str(), &send_address.sin_addr.s_addr);
	if (res < 0)
	{
		log("Error message with InetPton() : " + std::to_string(GETSOCKETERRNO()));
	}
	else if (res == 0)
	{
		log("Client address is not valid !");
	}
	send_address.sin_family = AF_INET;
	send_address.sin_port = htons(client.Port);
	int bytes = sendto(m_socket_emit, data.c_str(), static_cast<int>(data.length()), 0, 
		reinterpret_cast<sockaddr*>(&send_address), static_cast<int>(sizeof(struct sockaddr_in)));
	char log_buffer[4150];
	#ifdef IBM
	sprintf_s(log_buffer, 4150, "[%s:%d]<<<'%s'(%d byte(s))", client.Ip.c_str(), client.Port, data.c_str(), bytes);
	#else
	sprintf(log_buffer, "[%s:%d]<<<'%s'(%d byte(s))", client.ip.c_str(), client.port, data.c_str(), bytes);
	#endif
	log(std::string(log_buffer));
	if (bytes <= 0)
	{
		log("Error message : " + std::to_string(GETSOCKETERRNO()));
	}
	return bytes;
}

int UdpServer::BroadcastData(std::string const &data, u_short const port) const
{
	if(m_socket_beacon == INVALID_SOCKET) return 0;
	sockaddr_in send_address = {};
	send_address.sin_family = AF_INET;
	send_address.sin_port = htons(port);
	send_address.sin_addr.s_addr = INADDR_BROADCAST;
	//int res = inet_pton(AF_INET, cli.ip.c_str(), &send_address.sin_addr.s_addr);
	int bytes = sendto(m_socket_beacon, data.c_str(), static_cast<int>(data.length()), 0,
		reinterpret_cast<sockaddr*>(&send_address), static_cast<int>(sizeof(struct sockaddr_in)));
	if (bytes < 0)
	{
		log("There was an error during broadcast : " + std::to_string(GETSOCKETERRNO()));
	}
	char log_buffer[4150];
	#ifdef IBM
		sprintf_s(log_buffer, 4150, "BROADCAST[%d]<<<'%s'(%d byte(s))", port, data.c_str(), bytes);
	#else
		sprintf(log_buffer, "BROADCAST[%d]<<<'%s'(%d byte(s))", port, data.c_str(), bytes);
	#endif
	log(std::string(log_buffer));
	return 0;
}

void UdpServer::log(std::string const &data) const
{
	*m_logfile << GetCurrentDateTime() << "\t" << "UDP" << "\t" << data << "\n";
	m_logfile->flush();
}

int UdpServer::GetInboundPort() const
{
	return m_inPort;
}

int UdpServer::GetOutboundPort() const
{
	return m_outPort;
}

std::string UdpServer::GetLocalIp()
{
	return FindIp()[0];
}

std::string GetCurrentDateTime()
{
	tm* ltm;
	time_t now = time(0);
#ifdef IBM
	ltm = new tm;
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
	return { time };
}
