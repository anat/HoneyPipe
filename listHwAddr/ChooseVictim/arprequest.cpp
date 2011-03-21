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
ethheader ethHeader;
arpheader arpHeader;
      Packet p(this->craftETH(&ethHeader, 0x0806,
                src_hwaddr,
                dst_hwaddr));
      memset(dst_hwaddr, 0x00, 6);
      this->craftARP(&arpHeader,
                      src_hwaddr, (unsigned char *)&src_ip,
                      dst_hwaddr, (unsigned char *)&dst_ip);


      p.append(&arpHeader);


      std::cout << "Writing" << std::endl;
/*
      if (write(rsock, p.getBuffer(), sizeof(packet)) == -1)
        return 0;
      if (read(rsock, &packet, sizeof(packet)) == -1)
          std::cout << "failed" << std::endl;
      close(rsock);

        memcpy(foundMAC, packet.arp_head.ar_sha, 6);
*/
        return foundMAC;
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
