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

ARPRequest::ARPRequest()
{

}

uint8_t* ARPRequest::doRequest(QNetworkInterface const & interface, int src_ip, int dst_ip)
{
    uint8_t* foundMAC = new uint8_t[6];
    uint8_t src_hwaddr[6], dst_hwaddr[6];
    int rsock;
    //struct pack packet;

    std::cout << "Creating socket" << std::endl;
    if ((rsock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
        return 0;


    struct ifreq ifr;
    strncpy(ifr.ifr_name, interface.humanReadableName().toAscii(), IFNAMSIZ);
    if (ioctl(rsock, SIOCGIFHWADDR, &ifr) == -1)
        return 0;
    memcpy((char *)src_hwaddr, ((struct sockaddr *)&ifr.ifr_hwaddr)->sa_data, 6);



    struct sockaddr_ll sll;

    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_ifindex = interface.index();
    std::cout << "Binding" << std::endl;
    if (bind(rsock, (struct sockaddr *) &sll, sizeof(struct sockaddr_ll)) == -1)
    {
        std::cout << "Couldn't bind !" << std::endl;
        return 0;
    }

    memset(dst_hwaddr, 0xff, 6);



    ethheader eth_Header;
    arpheader* arp_Header = new arpheader();
    Packet p(this->craftETH(&eth_Header, 0x0806,
                            src_hwaddr,
                            dst_hwaddr));
    memset(dst_hwaddr, 0x00, 6);
    this->craftARP(arp_Header,
                   src_hwaddr, (unsigned char *)&src_ip,
                   dst_hwaddr, (unsigned char *)&dst_ip);


    p.append(arp_Header, sizeof(arpheader));


    std::cout << "Writing : " << p.Size << std::endl;
    if (write(rsock, p.getBuffer(), p.Size) == -1)
        return 0;


    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(rsock, &rfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000; // 100 ms

    int retval = select(rsock + 1, &rfds, NULL, NULL, &timeout);
    if (retval == -1)
    {
        perror("Failure");
        return NULL;
    }
    else if (retval)
    {
        if (read(rsock, p.getBuffer(), p.Size) == (sizeof(ethheader) + sizeof(arpheader)))
        {
            close(rsock);
            arp_Header = (arpheader*)(((char *)p.getBuffer()) + sizeof(ethheader));
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
            std::cout << "failed" << std::endl;
            close(rsock);
            return 0;
        }
    }


    //struct timeval tv;
    //struct timezone tz;
    //gettimeofday(&tv, &tz);
    //std::cout << tv.tv_usec << std::endl;

    std::cout << "Timeout" << std::endl;
    return 0;

}

arpheader* ARPRequest::craftARP(arpheader *arp,
                                uint8_t *srcmac, uint8_t *srcip,
                                uint8_t *dstmac, uint8_t *dstip)
{
    arp->ar_hrd = htons(1);
    arp->ar_pro = htons(0x0800);
    arp->ar_hln = 6;
    arp->ar_pln = 4;
    arp->ar_op = htons(ARP_REQUEST);
    memcpy(arp->ar_sha, srcmac, 6);
    memcpy(arp->ar_sip, srcip, 4);
    memcpy(arp->ar_tha, dstmac, 6);
    memcpy(arp->ar_tip, dstip, 4);
    return arp;
}

ethheader* ARPRequest::craftETH(ethheader *eth, uint16_t type,
                                uint8_t *srcmac, uint8_t *dstmac)
{
    eth->type = htons(type);
    memcpy(eth->ar_tha, dstmac, 6);
    memcpy(eth->ar_sha, srcmac, 6);
    return eth;
}
