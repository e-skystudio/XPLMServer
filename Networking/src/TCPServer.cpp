#ifdef WIN
#include "TCPServer.h"

TCPServer::TCPServer() :
    m_listenSocket(INVALID_SOCKET),
    m_wsa({ 0 }),
    m_serverAddr({ 0 }),
    m_maxReceivePacketSize(4096)
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
    fd_set ready_sockets;
    FD_ZERO(&ready_sockets);
    FD_SET(m_listenSocket, &ready_sockets);
    for (SOCKET cli : m_clients)
    {
        FD_SET(cli, &ready_sockets);
    }
    int res = select(FD_SETSIZE, &ready_sockets, NULL, NULL, &timeout);
    if (res < 0)
    {
        return readySocks;
    }
    else if (res == 0)
    {
        return readySocks;
    }
    if (FD_ISSET(m_listenSocket, &ready_sockets))
    {
        this->accept_connection();
    }
    for (SOCKET sock : m_clients)
    {
        if (FD_ISSET(sock, &ready_sockets))
        {
            readySocks.push_back(sock);
        }
    }
    return readySocks;
}

std::vector<std::string> TCPServer::RunOnce()
{
    std::vector<SOCKET> readySocket = this->Run();
    std::vector<std::string> dataIn;
    for (SOCKET s : readySocket)
    {
        int out{ 0 };
        std::string data = this->ReceiveData(s, &out);
        dataIn.push_back(data);
    }
    return dataIn;
}

int TCPServer::accept_connection()
{
    SOCKADDR_IN new_client_addr;
    int new_client_len = sizeof(SOCKADDR_IN);
    SOCKET new_client = accept(m_listenSocket, (sockaddr*)&new_client_addr, &new_client_len);
    if (new_client == INVALID_SOCKET)
        return 0x01;
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
        int bytes = send(so, data.c_str(), (int)data.length(), 0);
    }
}


void TCPServer::DeleteConnection(SOCKET socket)
{
    closesocket(socket);
    auto it = std::find(m_clients.begin(), m_clients.end(), socket);
    m_clients.erase(it);
}
#endif