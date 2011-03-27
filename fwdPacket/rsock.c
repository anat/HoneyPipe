#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "rsock.h"

int create_raw_socket()
{
  int fd;

  if ((fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
    {
      fprintf(stderr, "socket(): Error: %s.\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  return (fd);
}

int get_interface_id(int fd, char *name)
{
  struct ifreq ifr;

  strncpy(ifr.ifr_name, name, IFNAMSIZ);
  if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1)
    {
      fprintf(stderr, "ioctl(): Error: %s.\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  return (ifr.ifr_ifindex);
}

void get_interface_addr(int fd, char *name, uint8_t *hwaddr)
{
  struct ifreq ifr;

  strncpy(ifr.ifr_name, name, IFNAMSIZ);
  if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1)
    {
      fprintf(stderr, "ioctl(): Error: %s.\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  memcpy((char *)hwaddr, ((struct sockaddr *)&ifr.ifr_hwaddr)->sa_data, 6);
}

void bind_interface(int fd, int ifidx)
{
  struct sockaddr_ll sll;

  sll.sll_family = AF_PACKET;
  sll.sll_protocol = htons(ETH_P_ALL);
  sll.sll_ifindex = ifidx;
  if (bind(fd, (struct sockaddr *) &sll, sizeof(struct sockaddr_ll)) == -1)
    {
      fprintf(stderr, "bind(): Error: %s.\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
}

void promisc_mode(int fd, int ifidx, int s)
{
  struct packet_mreq pmr;

  memset(&pmr, 0, sizeof(struct packet_mreq));
  pmr.mr_ifindex = ifidx;
  pmr.mr_type = PACKET_MR_PROMISC;
  if (s)
    setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &pmr, sizeof(pmr));
  else
    setsockopt(fd, SOL_PACKET, PACKET_DROP_MEMBERSHIP, &pmr, sizeof(pmr));
}

uint32_t iptolong(char *s)
{
  uint32_t ip = 0;
  int i, j, nb, c = 3;
  int end = 0;

  for (i = 0; s[i] && !end; i++)
    {
      j = i;
      while (s[i] && s[i] != '.')
	i++;
      if (!s[i])
	end = 1;
      s[i] = '\0';
      nb = atoi(s + j);
      ip |= (nb << (c-- << 3));
    }
  return (ip);
}

int mypow(int n, int p)
{
  int on = n;

  if (!p)
    return (1);
  while (--p)
    n *= on;
  return (n);
}

int xtoi(char *s)
{
  int n, i, l;

  for (l = 0; s[l]; l++)
    {
      if (s[l] >= '0' && s[l] <= '9')
	s[l] -= '0';
      else if (s[l] >= 'a' && s[l] <= 'f')
	s[l] = s[l] - 'a' + 10;
      else
	break;
    }
  i = l;
  n = 0;
  while (--l >= 0)
    {
      n += s[l] * mypow(0x10, i - l - 1);
      s[l]++; /* /!\ Degeulasse */
    }
  return (n);
}

int mactoa(char *s, uint8_t *mac)
{
  int i;
  int n = 0;

  for (i = 0; s[i] && n < 6; i++)
    {
      mac[n++] = xtoi(s + i);
      while (s[i] && s[i] != ':')
	i++;
      if (!s[i])
	break;
    }
  if (n != 6 || s[i])
    return (-1);
  return (0);
}
