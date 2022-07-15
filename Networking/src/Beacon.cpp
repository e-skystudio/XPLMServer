#include "../include/Beacon.h"

Beacon::Beacon()
{
	m_logfile = new std::ofstream("XPLMServer_Network.log", std::ios::out);
	Log("Loging Started");
}

Beacon::~Beacon() = default;

int Beacon::Configure(std::string const& targetIp, unsigned short const outPort, bool const broadcast)
{
#ifdef IBM
    if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
        return 0x01;
#endif
    m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(m_socket == INVALID_SOCKET)
    {
        this->Log("Unable to configure socket : " + std::to_string(GETSOCKETERRNO()));
        return 0x02;
    }
    memset(&m_si_other, 0x00, sizeof(m_si_other));
    if (!broadcast)
    {
        if (inet_pton(AF_INET, targetIp.c_str(), &m_si_other.sin_addr.s_addr) < 0)
        {
            Log("Error message with InetPton() : " + std::to_string(GETSOCKETERRNO()));
            return 0x03;
        }
    }
    else
    {
        m_si_other.sin_addr.s_addr = INADDR_BROADCAST;
        int broadcast_enable = 1;
        constexpr int optlen = sizeof(int);
        if (setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char*>(&broadcast_enable), optlen) != 0)
        {
            Log("Error code is : '" + std::to_string(GETSOCKETERRNO()));
            return 0x03;
        }
    }
    m_si_other.sin_family = AF_INET;
    m_si_other.sin_port = htons(outPort);
    Log("[BEACON] Sending to : " + targetIp + ":" + std::to_string(outPort));
    return 0x00;

}

int Beacon::SendData(std::string const &data) const
{
    int bytes = sendto(
        m_socket,
        data.c_str(),
        static_cast<int>(data.length()),
        0x00,
        reinterpret_cast<const sockaddr*>(&m_si_other),
        m_slen);
    if(bytes < 0)
    {
        Log("ERROR WHILE SENDING BEACON : " + std::to_string(GETSOCKETERRNO()));
        return bytes;
    }
	char log_buffer[4150];
	#ifdef IBM
	sprintf_s(log_buffer, 4150, "[BEACON]>>>'%s'(%d byte(s))", data.c_str(), bytes);
	#else
	sprintf(log_buffer, "[BEACON]>>>'%s'(%d byte(s))", data.c_str(), bytes);
	#endif
	Log(std::string(log_buffer));
	return bytes;
}

void Beacon::Log(std::string const &data) const
{
#ifdef IBM
    OutputDebugString(reinterpret_cast<LPCWSTR>(data.c_str()));
#endif
	*m_logfile << GetCurrentDateTime() << "\t" << "BEACON" << "\t" << data << "\n";
	m_logfile->flush();
}

