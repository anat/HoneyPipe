#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

typedef char bool;
# define false 0
# define true  1

# define QUIT 1

void sigint_handler(int n);

struct ipheader {
  unsigned char ip_hl:4, ip_v:4; /* this means that each member is 4 bits */
  unsigned char ip_tos;
  unsigned short int ip_len;
  unsigned short int ip_id;
  unsigned short int ip_off;
  unsigned char ip_ttl;
  unsigned char ip_p;
  unsigned short int ip_sum;
  unsigned int ip_src;
  unsigned int ip_dst;
}; /* total ip header length: 20 bytes (=160 bits) */

struct icmpheader {
  unsigned char icmp_type;
  unsigned char icmp_code;
  unsigned short int icmp_cksum;
  /* The following data structures are ICMP type specific */
  unsigned short int icmp_id;
  unsigned short int icmp_seq;
}; /* total icmp header length: 8 bytes (=64 bits) */



int main()
{
  int fd;

  signal(SIGINT, sigint_handler);

  if ((fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
    {

      fprintf(stderr, "socket(): error: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

  return (EXIT_SUCCESS);
}

/* this function generates header checksums */
unsigned short csum (unsigned short *buf, int nwords)
{
  unsigned long sum;
  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}

void	push()
{
  struct ipheader ip;
  ip.ip_hl = 5; "size of packet header";
  ip.ip_v = 4;
  ip.ip_tos = 0; "priority";
  ip.len = 0x54; "size of packet header + all header + payload";
  ip.id = 0; "n Sequence";
  ip.ip_off = 0; " >fragment< offset";
  ip.ip_ttl = 255;
  ip.p = 1; "protocole en gros";
  ip.src = iptolong(strdup("10.0.2.15"));
  ip.dst = iptolong(strdup("192.168.0.254"));
  ip.ip_sum = 0;
  ip.ip_sum = csum ((unsigned short *) &ip, ip.len >> 1);

  struct icmpheader icmp;
  icmp.icmp_type = 8;
  icmp.icmp_code = 0;
  icmp.id = 1;
  icmp.seq = 137f;
  icmp.icmp_cksum = 0;
  icmp.icmp_cksum = csum ((unsigned short *) &icmp, iph->ip_len >> 1);

  char data[] = "\x";
"0000   08 00 20 ca 13 7f 00 01 ac 2c 81 4d 00 00 00 00
0010   cf 68 08 00 00 00 00 00 10 11 12 13 14 15 16 17
0020   18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 24 25 26 27
0030   28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 36 37";
}

int	iptolong(char *s)
{
  int ip = 0;
  int i, j, nb, c = 3;

  for (i = 0; s[i]; i++)
    {
      j = i;
      while (s[i] && s[i] != '.')
	i++;
      s[i] = '\0';
      nb = atoi(s + j);
      ip |= (nb << (c-- << 3));
    }
  return (ip);
}

void	catch(int fd)
{
  char buff[4096];
  int cc, i = 1;
  while ((cc = read(fd, buff, 4095)) != -1)
    {
      printf("%d: %d bytes received.\n", i, cc);
    }
}



void sigint_handler(int n)
{
  (void) n;
  exit(EXIT_SUCCESS);
}
