#include "packet.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

Packet::Packet() : Size(0), buffer(NULL), State(Route)
{
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

Packet * Packet::getPseudoIPTCPDATA()
{
    tcp* pTCP = static_cast<tcp*>(this->buffer);
    Packet* p = new Packet();

    // pseudo header ip
    p->append(&pTCP->ip_src, 4);
    p->append(&pTCP->ip_dst, 4);
    uint8_t a = 0;
    p->append(&a, 1);
    p->append(&pTCP->ip_p, 1);
    uint16_t b = htons(pTCP->ip_len) - (sizeof(ip) - sizeof(eth));
    b = htons(b);
    p->append(&b, 2);

    // tcp header and data...
    b = htons(b);
    p->append((uint8_t*)(this->buffer) + sizeof(ip), b);
    uint8_t odd = 0;
    if (p->Size % 2 != 0)
        p->append(&odd, 1);
    return p;
}

void Packet::computeChecksum()
{
    tcp* pTCP = static_cast<tcp*>(this->buffer);
    unsigned char * packet = ((unsigned char *)this->buffer) + sizeof(eth);

    // ip checksum calculation
    pTCP->ip_sum = 0;
    pTCP->ip_sum = this->checksum((uint16_t *)(packet), 20 >> 1);

    // tcp checksum calculation
    Packet * p = this->getPseudoIPTCPDATA();
    checktcp * test = (checktcp*)(p->getBuffer());
    test->tcp.check = 0;
    pTCP->check = checksum((uint16_t*)p->getBuffer(), p->Size >> 1);
    delete p;
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

void ip::craftIP(uint8_t *srcmac, uint32_t srcip,
                 uint8_t *dstmac, uint32_t dstip)
{
    this->craftETH(0x0800, srcmac, dstmac);
    this->ip_hl = 5;
    this->ip_v = 4;
    this->ip_tos = 0;
    this->ip_len = htons(40 + 20); // 16 without options ??
    this->ip_id = htons(1337);
    this->ip_off = 0x40;
    this->ip_ttl = 255;
    this->ip_p = 0; // to set later before compute the checksum
    this->ip_sum = 0;
    this->ip_src = srcip;
    this->ip_dst = dstip;
}

int Packet::getSizeOfData()
{
    tcp* pTCP = static_cast<tcp*>(this->buffer);

    return (pTCP->ip_len - (pTCP->ip_hl << 2) - (pTCP->doff << 2));
}

int Packet::getTCPHeaderSize()
{
    tcp* pTCP = static_cast<tcp*>(this->buffer);

    return (pTCP->doff << 2);
}

void tcp::craftTCP(uint8_t *srcmac, uint32_t srcip,
                   uint8_t *dstmac, uint32_t dstip, uint16_t srcPort, uint16_t dstPort, uint32_t seq, uint32_t ack)
{

    this->craftIP(srcmac, srcip, dstmac, dstip);
    this->ip_p = IPPROTO_TCP;
    this->ip_sum = this->checksumIP((uint16_t *)( ((unsigned char *)this) + sizeof(eth)), 20 >> 1);

    this->source = htons(srcPort);
    this->dest = htons(dstPort);
    this->seq = htonl(seq);
    this->ack_seq = htonl(ack);
    this->res1 = 0;
    this->doff = 0x5;
    this->fin = 0;
    this->syn = 0;
    this->rst = 0;
    this->psh = 0;
    this->ack = 0;
    this->urg = 0;
    this->ecn = 0;
    //this->res2 = 0;
    this->window = htons(64);
    this->check = 0;
    this->urg_ptr = 0;
    //this->options = 0;
}
