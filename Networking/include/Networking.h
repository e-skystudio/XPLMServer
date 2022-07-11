#pragma  once
#ifdef IBM
#include <WinSock2.h>
#include <iostream>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

#else
#include <unistd.h>
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <arpa/inet.h>

#endif
#include <string>
#include <vector>

std::vector<std::string> FindIp();