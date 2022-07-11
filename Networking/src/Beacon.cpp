#include "../include/Beacon.h"

Beacon::Beacon():
    m_outPort(0),
	m_ip(""),
    m_socket(INVALID_SOCKET),
    m_targetAddress(0)

{
	m_logfile = new std::ofstream("XPLMServer_Network.log", std::ios::out);
	log("Loging Started");
}

Beacon::~Beacon() = default;

int Beacon::Configure(std::string const targetIp, unsigned short const outPort, bool const broadcast)
{
    m_ip = targetIp;
    m_outPort = outPort;
    addrinfo hints = {};
    hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
    struct addrinfo* addrInfo = {};
    int _ = getaddrinfo(0, std::to_string(m_outPort).c_str(), &hints, &addrInfo);
    if(!broadcast)
    {
        int res = inet_pton(AF_INET, targetIp.c_str(), &m_targetAddress->sin_addr.s_addr);
        if (res < 0)
        {
            log("Error message with InetPton() : " + std::to_string(GETSOCKETERRNO()));
        }
        else if (res == 0)
        {
            log("Client address is not valid !");
        }
    }
    else
    {
        m_targetAddress->sin_addr.s_addr = INADDR_BROADCAST;
        int broadcast_enable = 1;
        constexpr int optlen = sizeof(int);
        int const res = setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&broadcast_enable), optlen);
        if (res != 0)
		{
			log("Error code is : '" + std::to_string(GETSOCKETERRNO()));
			return 0x03;
		}
    }
    m_targetAddress->sin_family = addrInfo->ai_family;
    m_targetAddress->sin_port = htons(m_outPort);
    m_socket = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if(m_socket == INVALID_SOCKET){
        this->log("Unable to configure socket : " + std::to_string(GETSOCKETERRNO()));
        return 0x01;
    }
    return 0x00;
}

int Beacon::SendData(std::string const &data) const
{
	int bytes = sendto(m_socket, data.c_str(), static_cast<int>(data.length()), 0, 
		(sockaddr*)&m_targetAddress, static_cast<int>(sizeof(struct sockaddr_in)));

	char log_buffer[4150];
	#ifdef IBM
	sprintf_s(log_buffer, 4150, "[BEACON]>>>'%s'(%d byte(s))", data.c_str(), bytes);
	#else
	sprintf(log_buffer, "[BEACON]>>>'%s'(%d byte(s))", data.c_str(), bytes);
	#endif
	log(std::string(log_buffer));
	if (bytes <= 0)
	{
		log("Error message : " + std::to_string(GETSOCKETERRNO()));
	}
	return bytes;
}

void Beacon::log(std::string const &data) const
{
	*m_logfile << GetCurrentDateTime() << "\t" << "BEACON" << "\t" << data << "\n";
	m_logfile->flush();
}

