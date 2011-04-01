#include "arprequest.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#include "packet.h"
#include <sys/select.h>
#include <sys/time.h>
#include "rawsocket.h"
#include <stdlib.h>

ARPRequest::ARPRequest()
{

}

uint8_t* ARPRequest::doRequest(RAWSocket & s, QNetworkInterface const & interface, int src_ip, int dst_ip)
{
    uint8_t* foundMAC = new uint8_t[6];
    uint8_t src_hwaddr[6], dst_hwaddr[6];

    struct ifreq ifr;
    strncpy(ifr.ifr_name, interface.humanReadableName().toAscii(), IFNAMSIZ);
    if (ioctl(s.Handler, SIOCGIFHWADDR, &ifr) == -1)
        return 0;
    memcpy((char *)src_hwaddr, ((struct sockaddr *)&ifr.ifr_hwaddr)->sa_data, 6);

    memset(dst_hwaddr, 0xff, 6);

    arp* arp_Header = new arp;
    arp_Header->craftETH(0x0806, src_hwaddr, dst_hwaddr);

    memset(dst_hwaddr, 0x00, 6);
    arp_Header->craftARP(src_hwaddr, (unsigned char *)&src_ip,
                   dst_hwaddr, (unsigned char *)&dst_ip);

    Packet p;
    p.append(arp_Header, sizeof(arp));

    if (s.Write(p) == -1)
        return 0;

    int retval = s.Poll(50000);
    if (retval == -1)
    {
        perror("Failure");
        return NULL;
    }
    else if (retval)
    {
        if (s.Read(p) == (sizeof(arp)))
        {
            arp_Header = (arp*)((char *)p.getBuffer());
            if (arp_Header->ar_op != htons(ARP_REPLY))
            {
                std::cout << "Not arp REPLY" << std::endl;
                return 0;
            }
            if (memcmp(src_hwaddr,&(arp_Header->ar_tha), 6) != 0)
            {
                std::cout << "Bad Target MAC" << std::endl;
                return 0;
            }
            memcpy(foundMAC, arp_Header->ar_sha, 6);
            return foundMAC;
        }
        else
        {
            std::cout << "Timeout" << std::endl;
            return 0;
        }
    }
    std::cout << "Exit" << std::endl;

    //struct timeval tv;
    //struct timezone tz;
    //gettimeofday(&tv, &tz);
    //std::cout << tv.tv_usec << std::endl;
    return 0;

}




