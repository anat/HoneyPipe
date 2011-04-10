#ifndef PACKET_H
# define PACKET_H
# include <stdlib.h>
# include <iostream>
# include <string.h>
# include <stdint.h>
# include <arpa/inet.h>
// For QT's Funny parser to work
//# define __attribute__(spec)
#include <netinet/tcp.h>

class Packet
{
private:
    void *buffer;
    uint16_t checksum(uint16_t *buf, int nwords);
public:
    Packet();
    ~Packet();
    template<typename T>
    inline int append(T* data, int size)
    {
        this->buffer = realloc(this->buffer, this->Size + size);
        memcpy(((char *)this->buffer) + this->Size, data, size);
        this->Size += size;
        return this->Size;
    }
    inline void reduce(int size)
    {
        this->Size -= size;
        this->buffer = realloc(this->buffer, this->Size);
    }
    void computeChecksum();
    void * getBuffer();
    Packet * getPseudoIPTCPDATA();
    int getSizeOfData();
    int getTCPHeaderSize();
    uint32_t Size;
    bool Store;
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
  void craftIP(uint8_t *srcmac, uint32_t srcip, uint8_t *dstmac, uint32_t dstip);

  uint16_t checksumIP(uint16_t *buf, int nwords)
  {
      uint32_t sum;
      for (sum = 0; nwords > 0; nwords--)
          sum += *buf++;
      sum = (sum >> 16) + (sum & 0xffff);
      sum += (sum >> 16);
      return ~sum;
  }
} __attribute__ ((packed));




struct tcp : public ip
{
  uint16_t source;
  uint16_t dest;
  uint32_t seq;
  uint32_t ack_seq;
  uint16_t res1:4;
  //uint16_t doff:4;
  uint16_t doff:6;
  uint16_t fin:1;
  uint16_t syn:1;
  uint16_t rst:1;
  uint16_t psh:1;
  uint16_t ack:1;
  uint16_t urg:1;
  //uint16_t res2:2;
  uint16_t window;
  uint16_t check;
  uint16_t urg_ptr;
  //uint32_t options;
  void craftTCP(uint8_t *srcmac, uint32_t srcip, uint8_t *dstmac, uint32_t dstip);
} __attribute__ ((packed));

/** TCP Header Format

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |          Source Port          |       Destination Port        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                        Sequence Number                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Acknowledgment Number                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Data |           |U|A|P|R|S|F|                               |
   | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
   |       |           |G|K|H|T|N|N|                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Checksum            |         Urgent Pointer        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Options                    |    Padding    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                             data                              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

struct checktcp {
    uint32_t source;
    uint32_t destination;
    unsigned char useless;
    unsigned char protocol;
    uint16_t length;
    struct tcphdr tcp;
} __attribute__ ((packed));

struct tcpack : public tcp
{
    static tcpack* craftACK(){return NULL;}
};

#endif // PACKET_H
