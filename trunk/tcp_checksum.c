#include <stdint.h>
#include <stdio.h>
#include <arpa/inet.h>

char *pqt = /* Checksum : 0x62fb */
  /* Header : 32 bytes */
  //"\xb2\x35\x10\x92\x4e\x9f\xa9\x6d\x74\xf5\xc5\x27\x80\x18\x00\x40"
  //"\x00\x00\x00\x00"//\x01\x01\x08\x0a\x00\x56\x6b\xcc\x00\x00\x06\x10"
  /* Data   : 32 bytes */
  //"\x75\x73\x65\x72\x5f\x63\x6d\x64\x20\x6d\x73\x67\x5f\x75\x73\x65"
  //"\x72\x20\x3a\x38\x37\x31\x20\x6d\x73\x67\x20\x74\x65\x73\x74\x0a";
  /* Checksum : 0x8b27 */
  "\x34\x10\xc4\xbb\xf7\xb6\xcf\x19\xd8\x25\x42\xe9\x50\x18\xfd\x10"
  "\x00\x00\x00\x00"
  "\x46\x68\xd7\xb1";

  char *pip = "\x00\x2c\x00\x06\x58\xa2\x15\x7f\xc0\xa8\x00\x01";

unsigned short tcp_sum_calc(unsigned short len_tcp, unsigned short *src_addr, unsigned short *dest_addr, unsigned short *buff);
unsigned short in_chksum_tcp(  unsigned short *h, unsigned short * d, int dlen );

uint16_t tcpChecksum(uint16_t *buf, int nwords)
{
  uint8_t ip_p = 0x06;
  uint16_t ip_len = 24;
  uint32_t ip_dst = 0x05ff05a3, ip_src = 0x0100a8c0;
  uint32_t sum = 0;

  sum += htons(ip_len);
  //sum += htons(52);
  sum += htons((uint16_t)ip_p);
  sum += (*((uint16_t*)&ip_src));
  //printf("src[0]: %x\n",htons(*((uint16_t*)&ip_src)));
  sum += (*(((uint16_t*)&ip_src)+1));
  //printf("src[1]: %x\n", htons(*(((uint16_t*)&ip_src)+1)));
  sum += (*((uint16_t*)&ip_dst));
  //printf("dst[0]: %x\n", htons(*((uint16_t*)&ip_dst)));
  sum += (*(((uint16_t*)&ip_dst)+1));
  //printf("dst[1]: %x\n", htons(*(((uint16_t*)&ip_dst)+1)));

  for (; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}

int main()
{
  uint16_t sum = htons(tcpChecksum((uint16_t *)pqt, 24 >> 1));
  printf("Checksum : 0x%x\n", sum);
  //uint32_t ip_dst = 0x05ff05a3, ip_s = 0x0100a8c0;  
  //sum = tcp_sum_calc(64, (uint16_t*)&ip_s, (uint16_t*)&ip_dst, (uint16_t *)pqt);
  sum = in_chksum_tcp((unsigned short *)pip, (unsigned short *)pqt, 24);
  printf("His Checksum : 0x%x\n", sum);
  return (0);
}



unsigned short in_chksum_tcp(  unsigned short *h, unsigned short * d, int dlen )
{
  unsigned int cksum;
  unsigned short answer=0;

  /* PseudoHeader must have 12 bytes */
  cksum  = h[0];
  cksum += h[1];
  cksum += h[2];
  cksum += h[3];
  cksum += h[4];
  cksum += h[5];

  /* TCP hdr must have 20 hdr bytes */
  cksum += d[0];
  cksum += d[1];
  cksum += d[2];
  cksum += d[3];
  cksum += d[4];
  cksum += d[5];
  cksum += d[6];
  cksum += d[7];
  cksum += d[8];
  cksum += d[9];

  dlen  -= 20; /* bytes   */
  d     += 10; /* short's */

  while(dlen >=32)
    {
      cksum += d[0];
      cksum += d[1];
      cksum += d[2];
      cksum += d[3];
      cksum += d[4];
      cksum += d[5];
      cksum += d[6];
      cksum += d[7];
      cksum += d[8];
      cksum += d[9];
      cksum += d[10];
      cksum += d[11];
      cksum += d[12];
      cksum += d[13];
      cksum += d[14];
      cksum += d[15];
      d     += 16;
      dlen  -= 32;
    }

  while(dlen >=8)
    {
      cksum += d[0];
      cksum += d[1];
      cksum += d[2];
      cksum += d[3];
      d     += 4;
      dlen  -= 8;
    }

  while(dlen > 1)
    {
      cksum += *d++;
      dlen  -= 2;
    }

  if( dlen == 1 )
    {
      /* printf("new checksum odd byte-packet\n"); */
      *(unsigned char*)(&answer) = (*(unsigned char*)d);

      /* cksum += (u_int16_t) (*(u_int8_t*)d); */

      cksum += answer;
    }
  cksum  = (cksum >> 16) + (cksum & 0x0000ffff);
  cksum += (cksum >> 16);
  return (unsigned short)(~cksum);
}





unsigned short tcp_sum_calc(unsigned short len_tcp, unsigned short *src_addr, unsigned short *dest_addr, unsigned short *buff)

{
  unsigned short prot_tcp = 6;
  long sum;
  int i;

  sum = 0;

  /* Check if the tcp length is even or odd.  Add padding if odd. */
  if((len_tcp % 2) == 1){
    buff[len_tcp] = 0;  // Empty space in the ip buffer should be 0 anyway.
    len_tcp += 1; // incrase length to make even.
  }
  /* add the pseudo header */
  sum += ntohs(src_addr[0]);
  sum += ntohs(src_addr[1]);
  sum += ntohs(dest_addr[0]);
  sum += ntohs(dest_addr[1]);
  sum += len_tcp; // already in host format.
  sum += prot_tcp; // already in host format. 

  /* 
   * calculate the checksum for the tcp header and payload
   * len_tcp represents number of 8-bit bytes, 
   * we are working with 16-bit words so divide len_tcp by 2. 
   */

  for(i=0;i<(len_tcp/2);i++){
    sum += ntohs(buff[i]);
  }
  // keep only the last 16 bits of the 32 bit calculated sum and add the carries
  sum = (sum & 0xFFFF) + (sum >> 16);
  sum += (sum >> 16);

  // Take the bitwise complement of sum
  sum = ~sum;
  return htons(((unsigned short) sum));

}
