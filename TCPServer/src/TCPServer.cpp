#include "../include/TCPServer.h"

TCPServer::TCPServer() :
    m_listenSocket(INVALID_SOCKET), m_wsa({ 0 }),
    m_serverAddr({ 0 }), m_current_socket(),
    m_maxsock(0), m_minsock(0), m_ready_socket()
{
}

int TCPServer::Initialize(std::string ip, unsigned short port)
{
    if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0)
        return 0x01;
    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET)
    {
        int error = WSAGetLastError();
        WSACleanup();
        return 0x02;
    }
    m_serverAddr.sin_family = AF_INET;
    m_serverAddr.sin_addr.s_addr = INADDR_ANY;
    m_serverAddr.sin_port = htons(port);
    if (bind(m_listenSocket, (SOCKADDR*)&m_serverAddr, sizeof(m_serverAddr)) == SOCKET_ERROR)
    {
        closesocket(m_listenSocket);
        int error = WSAGetLastError();
        WSACleanup();
        return 0x04;
    }
    if (listen(m_listenSocket, 5) != EXIT_SUCCESS)
    {
        int error = WSAGetLastError();
        WSACleanup();
        return 0x05;
    }
    return 0;
}

std::vector<SOCKET> TCPServer::Run()
{
    std::vector<SOCKET> readySocks;
    timeval timeout({ 0, 10000 });
    FD_ZERO(&m_current_socket);
    FD_SET(m_listenSocket, &m_current_socket);
    m_minsock = (unsigned int)m_listenSocket;
    m_maxsock = (unsigned int)m_listenSocket;
    for (SOCKET cli : m_clients)
    {
        FD_SET(cli, &m_current_socket);
        if ((int)cli > m_maxsock)
            m_maxsock = (unsigned int)cli;
        if ((int)cli < m_minsock)
            m_minsock = (unsigned int)cli;
    }
    m_ready_socket = m_current_socket;
    int res = select(FD_SETSIZE, &m_ready_socket, NULL, NULL, &timeout);
    if (res < 0)
    {
        return readySocks;
    }
    else if (res == 0)
    {
        return readySocks;
    }

    for (unsigned int i = m_minsock; i <= m_maxsock; i++)
    {
        if (FD_ISSET(i, &m_ready_socket))
        {
            if (i == m_listenSocket)
            {
                this->accept_connection();
            }
            else
            {
                readySocks.push_back((SOCKET)i);
            }
        }
    }
    return readySocks;
}

int TCPServer::accept_connection()
{
    SOCKADDR_IN new_client_addr;
    int new_client_len = sizeof(SOCKADDR_IN);
    SOCKET new_client = accept(m_listenSocket, (sockaddr*)&new_client_addr, &new_client_len);
    if (new_client == INVALID_SOCKET)
        return 0x01;
    FD_SET(new_client, &m_current_socket);

    m_clients.push_back(new_client);
    return 0;
}

std::string TCPServer::ReceiveData(const SOCKET socket, int* OutBytes)
{
    char* buffer = new char[4098];
    int bytes = recv(socket, buffer, 4098, 0);
    std::string data_received{};
    if (bytes < 0)
    {
        *OutBytes = bytes;
        return data_received;
    }
    data_received = std::string(buffer);
    data_received.resize(bytes);
    return data_received;

}

void TCPServer::BroadcastData(std::string data)
{
    for (SOCKET so : m_clients)
    {
        int bytes = send(so, data.c_str(), data.length(), 0);
    }
}


void TCPServer::DeleteConnection(SOCKET socket)
{
    closesocket(socket);
    auto it = std::find(m_clients.begin(), m_clients.end(), socket);
    m_clients.erase(it);
}
