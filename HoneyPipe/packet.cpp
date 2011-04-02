#include "packet.h"
#include <string.h>
#include <stdlib.h>


Packet::Packet()
{
    this->Size = 0;
    this->buffer = NULL;
}

Packet::~Packet()
{
    if (this->buffer)
        free(this->buffer);
}

void * Packet::getBuffer()
{
    return this->buffer;
}

uint16_t Packet::checksum(uint16_t *buf, int nwords)
{
    uint16_t sum;

    for (sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

void eth::craftETH(uint16_t type, uint8_t *srcmac, uint8_t *dstmac)
{
    this->type = htons(type);
    memcpy(this->ar_tha, dstmac, 6);
    memcpy(this->ar_sha, srcmac, 6);
}

void arp::craftARP(uint8_t *srcmac, uint8_t *srcip,
                   uint8_t *dstmac, uint8_t *dstip)
{
    this->ar_hrd = htons(1);
    this->ar_pro = htons(0x0800);
    this->ar_hln = 6;
    this->ar_pln = 4;
    this->ar_op = htons(ARP_REQUEST);
    memcpy(this->ar_sha, srcmac, 6);
    memcpy(this->ar_sip, srcip, 4);
    memcpy(this->ar_tha, dstmac, 6);
    memcpy(this->ar_tip, dstip, 4);
}

void ip::craftIP()
{

}

void tcp::craftTCP()
{

}
