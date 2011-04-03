#include "packet.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
    uint32_t sum;

    for (sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

void Packet::computeChecksum()
{
    tcp* pTCP = static_cast<tcp*>(this->buffer);
    unsigned char * packet = ((unsigned char *)this->buffer) + sizeof(eth);

    pTCP->ip_sum = 0;
    pTCP->ip_sum = this->checksum((uint16_t *)(packet), 20 >> 1);

 /*
    csum_tcpudp_magic(saddr,daddr,len,IPPROTO_TCP,base);

    static inline __sum16 csum_tcpudp_magic(__be32 saddr, __be32 daddr,
                                            unsigned short len,
                                            unsigned short proto,
                                            __wsum sum)
    {
            return csum_fold(csum_tcpudp_nofold(saddr, daddr, len, proto, sum));
    }
static inline __sum16 csum_fold(__wsum sum)
{
        asm("addl %1, %0                ;\n"
            "adcl $0xffff, %0   ;\n"
            : "=r" (sum)
            : "r" ((__force u32)sum << 16),
              "0" ((__force u32)sum & 0xffff0000));
        return (__force __sum16)(~(__force u32)sum >> 16);
}
static inline __wsum csum_tcpudp_nofold(__be32 saddr, __be32 daddr,
                                        unsigned short len,
                                        unsigned short proto,
                                        __wsum sum)
{
        asm("addl %1, %0        ;\n"
            "adcl %2, %0        ;\n"
            "adcl %3, %0        ;\n"
            "adcl $0, %0        ;\n"
            : "=r" (sum)
            : "g" (daddr), "g"(saddr),
              "g" ((len + proto) << 8), "0" (sum));
        return sum;
}



    */
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
