#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

class TCPServer
{
public:
    TCPServer();
    int Initialize(std::string ip, unsigned short port);
    std::vector<SOCKET> Run();
    std::vector<std::string> RunOnce();
    void DeleteConnection(SOCKET socket);
    std::string ReceiveData(const SOCKET socket, int* OutBytes);
    void BroadcastData(std::string data);
protected:
    WSADATA m_wsa;
    SOCKET m_listenSocket;
    SOCKADDR_IN m_serverAddr;
    std::vector<SOCKET> m_clients;
    unsigned int m_maxReceivePacketSize;
    char separator = 0x01;

protected:
    int accept_connection();
};

