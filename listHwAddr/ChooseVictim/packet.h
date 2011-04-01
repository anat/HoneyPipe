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
public:
    Packet();
    ~Packet();

    template<typename T>
    int append(T* buffer, int size)
    {
        this->buffer = realloc(this->buffer, this->Size + size);
        memcpy(((char *)this->buffer) + this->Size, buffer, size);
        this->Size += size;
        std::cout << "Packet Size = " << this->Size << std::endl;
        return this->Size;
    }
    uint16_t checksum(uint16_t *buf, int nwords);
    void * getBuffer();
    int Size;
};

struct eth
{
  uint8_t  ar_tha[6];
  uint8_t  ar_sha[6];
  uint16_t type;
  void craftETH(uint16_t type, uint8_t *srcmac, uint8_t *dstmac);
};

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
};


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
};




struct tcp : ip
  {
    u_int16_t source;
    u_int16_t dest;
    u_int32_t seq;
    u_int32_t ack_seq;
    u_int16_t res1:4;
    u_int16_t doff:4;
    u_int16_t fin:1;
    u_int16_t syn:1;
    u_int16_t rst:1;
    u_int16_t psh:1;
    u_int16_t ack:1;
    u_int16_t urg:1;
    u_int16_t res2:2;
    u_int16_t window;
    u_int16_t check;
    u_int16_t urg_ptr;
    void craftTCP();
};



#endif // PACKET_H
