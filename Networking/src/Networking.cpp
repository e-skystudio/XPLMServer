#include "../include/Networking.h"

std::vector<std::string> findIp()
{
    std::vector<std::string> ips;
    #ifndef IBM
    struct ifaddrs * addrs, * ifloop; 
	char buf[64];
	struct sockaddr_in *s4;
	const char * en0 = "en0";
	getifaddrs(&addrs);
    in_addr_t loopback = inet_addr("127.0.0.1");
	for (ifloop = addrs; ifloop != NULL; ifloop = ifloop->ifa_next)
	{
        if(ifloop->ifa_addr->sa_family != AF_INET) continue;
		s4 = (struct sockaddr_in *)(ifloop->ifa_addr);
        if(s4->sin_addr.s_addr == loopback) continue;
		inet_ntop(ifloop->ifa_addr->sa_family, (void *)&(s4->sin_addr), buf, sizeof(buf)) == NULL;
        ips.push_back(std::string(buf));
	}
    freeifaddrs(addrs);
    #endif
    return ips;
}