#ifndef PACKET_H
# define PACKET_H
# include <stdlib.h>
# include <iostream>
# include <string.h>
# include <stdint.h>
# include <arpa/inet.h>

class Packet
{
private:
    void *buffer;
    uint16_t checksum(uint16_t *buf, int nwords);
public:
    Packet();
    ~Packet();

    template<typename T>
    int append(T* data, int size)
    {
        this->buffer = realloc(this->buffer, this->Size + size);
        memcpy(((char *)this->buffer) + this->Size, data, size);
        this->Size += size;
        //std::cout << "Packet Size = " << this->Size << std::endl;
        return this->Size;
    }
    void computeChecksum();
    void * getBuffer();
    uint32_t Size;
};

struct eth
{
  uint8_t  ar_tha[6];
  uint8_t  ar_sha[6];
  uint16_t type;
  void craftETH(uint16_t type, uint8_t *srcmac, uint8_t *dstmac);
} __attribute__ ((packed));

# define ARP_REQUEST   1        /* ARP request.  */
# define ARP_REPLY     2        /* ARP reply.  */

struct arp : public eth
{
  uint16_t ar_hrd;      /* Format of hardware address.  */
  uint16_t ar_pro;      /* Format of protocol address.  */
  uint8_t  ar_hln;      /* Length of hardware address.  */
  uint8_t  ar_pln;      /* Length of protocol address.  */
  uint16_t ar_op;       /* ARP opcode (command).  */

  uint8_t  ar_sha[6];   /* Sender hardware address.  */
  uint8_t  ar_sip[4];   /* Sender IP address.  */
  uint8_t  ar_tha[6];   /* Target hardware address.  */
  uint8_t  ar_tip[4];   /* Target IP address.  */
  void craftARP(uint8_t *srcmac, uint8_t *srcip, uint8_t *dstmac, uint8_t *dstip);
} __attribute__ ((packed));


//enum {
 // IPPROTO_IP = 0,               /* Dummy protocol for TCP               */
//  IPPROTO_ICMP = 1,             /* Internet Control Message Protocol    */
//  IPPROTO_TCP = 6,              /* Transmission Control Protocol        */
 // IPPROTO_UDP = 17,             /* User Datagram Protocol               */
 // IPPROTO_RAW = 255,         /* Raw IP packets                       */
 // IPPROTO_MAX
//};


struct ip : public eth
{
  uint8_t  ip_hl:4;
  uint8_t  ip_v:4;
  uint8_t  ip_tos;
  uint16_t ip_len;
  uint16_t ip_id;
  uint16_t ip_off;
  uint8_t  ip_ttl;
  uint8_t  ip_p;
  uint16_t ip_sum;
  uint32_t ip_src;
  uint32_t ip_dst;
  bool isTCP() {return (this->ip_p == IPPROTO_TCP);}
  void craftIP();
} __attribute__ ((packed));




struct tcp : public ip
{
  uint16_t source;
  uint16_t dest;
  uint32_t seq;
  uint32_t ack_seq;
  uint16_t res1:4;
  uint16_t doff:4;
  uint16_t fin:1;
  uint16_t syn:1;
  uint16_t rst:1;
  uint16_t psh:1;
  uint16_t ack:1;
  uint16_t urg:1;
  uint16_t res2:2;
  uint16_t window;
  uint16_t check;
  uint16_t urg_ptr;
  void craftTCP();
} __attribute__ ((packed));



#endif // PACKET_H
