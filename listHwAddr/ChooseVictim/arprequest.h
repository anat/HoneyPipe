#ifndef ARPREQUEST_H
#define ARPREQUEST_H

#include <QtNetwork/QNetworkInterface>
#include <stdint.h>
#include "arp.h"
#include "eth.h"
#include <string>

struct pack
{
  ethheader eth_head;
  arpheader arp_head;
};// __attribute__ ((packed));


class ARPRequest
{
public:
    ARPRequest();
    uint8_t* doRequest(QNetworkInterface const & interface, int src_ip, int dst_ip);
private:
    void craftARP(arpheader *arp,
                   uint8_t *srcmac, uint8_t *srcip,
                   uint8_t *dstmac, uint8_t *dstip);
    void craftETH(ethheader *eth, uint16_t type,
                              uint8_t *srcmac, uint8_t *dstmac);

};

#endif // ARPREQUEST_H
