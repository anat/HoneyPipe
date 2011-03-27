#include "honeyPipe.h"

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

int main()
{
  int fd;

  signal(SIGINT, sigint_handler);

  if ((fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
    {
      fprintf(stderr, "socket(): error: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

  //catch(fd);
  push(fd);

  close(fd);
  return (EXIT_SUCCESS);
}

uint16_t checksum(uint16_t *buf, int nwords)
{
  uint16_t sum;

  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}

void push(int fd)
{
  t_ipheader ip;
  t_icmpheader icmp;

  ip.ip_hl  = 5;
  ip.ip_v   = 4;
  ip.ip_tos = 0;
  ip.ip_len = 0x54;
  ip.ip_id  = 0;
  ip.ip_off = 0;
  ip.ip_ttl = 255;
  ip.ip_p   = 1;
  ip.ip_src = iptolong(strdup("10.0.2.15"));
  ip.ip_dst = iptolong(strdup("192.168.0.254"));
  ip.ip_sum = 0;
  ip.ip_sum = checksum((unsigned short *) &ip, ip.ip_len >> 1);

  icmp.icmp_type  = 8;
  icmp.icmp_code  = 0;
  icmp.icmp_id    = 1;
  icmp.icmp_seq   = 0x137f;
  icmp.icmp_cksum = 0;
  //icmp.icmp_cksum = checksum((unsigned short *) &icmp, iph->ip_len >> 1);

  /*char data[] = "\x";
  "0000   08 00 20 ca 13 7f 00 01 ac 2c 81 4d 00 00 00 00
  0010   cf 68 08 00 00 00 00 00 10 11 12 13 14 15 16 17
  0020   18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 24 25 26 27
  0030   28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 36 37";*/
}

/********************************************************/
/* PING EN DUR QUI FONCTIONNE (MACs, IPs, checksum, if) */
/********************************************************/
/*  char *buff =
    "\x00\x07\xcb\x0c\x45\xa7\x08\x00\x27\xe5\x3b\xaf\x08\x00\x45\x00"
    "\x00\x54\x00\x00\x40\x00\x40\x01\x15\x3d\xc0\xa8\x00\x04\xd1\x55"
    "\x93\x6a\x08\x00\x20\xca\x13\x7f\x00\x01\xac\x2c\x81\x4d\x00\x00"
    "\x00\x00\xcf\x68\x08\x00\x00\x00\x00\x00\x10\x11\x12\x13\x14\x15"
    "\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23\x24\x25"
    "\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33\x34\x35\x36\x37";
  struct sockaddr_ll sll;

  memset(&sll, 0, sizeof(struct sockaddr_ll));
  sll.sll_family = AF_PACKET;
  memcpy(sll.sll_addr, "\x08\x00\x27\xe5\x3b\xaf", 6);
  sll.sll_halen = 6;
  sll.sll_ifindex = 2;
  int cc = sendto(fd, buff, 98, 0, (struct sockaddr *) &sll, sizeof(struct sockaddr_ll));
  if (cc == -1)
    {
      printf("sendto: %s\n", strerror(errno));
      exit(1);
    }
  printf("sendto: %d\n", cc);*/

  /*struct packet_mreq pmr;
  memset(&pmr, 0, sizeof(struct packet_mreq));
  pmr.mr_ifindex = 0;
  pmr.mr_type = PACKET_MR_PROMISC;
  setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &pmr, sizeof(struct packet_mreq));*/
  /*setsockopt(fd, SOL_PACKET, PACKET_DROP_MEMBERSHIP, &pmr, sizeof(struct packet_mreq));*/
/*************************************************/
/*************************************************/

int iptolong(char *s)
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

void catch(int fd)
{
  char buff[4096];
  int cc, num = 1;

  while ((cc = read(fd, buff, 4095)) != -1)
    {
      printf("%d: %d bytes received.\n", num, cc);
      num++;
    }
}

void sigint_handler(int n)
{
  (void) n;
  exit(EXIT_SUCCESS);
}
