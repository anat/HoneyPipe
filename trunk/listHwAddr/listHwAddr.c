#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "network_header.h"

void sigint(int n);
int  create_raw_socket();
int  get_interface_id(int fd, char *name);
void get_interface_addr(int fd, char *name, uint8_t *hwaddr);
void bind_interface(int fd, int ifidx);

int rsock;

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


struct pack
{
  eth_t eth_head;
  arp_t arp_head;
} __attribute__ ((packed));

int main(int argc, char **argv)
{
  uint8_t buff[4096];
  uint8_t src_hwaddr[6], dst_hwaddr[6];
  int n = 1, cc;
  int ifidx;
  struct pack packet;
  int src_ip, dst_ip;

  if (argc != 2)
    {
      fprintf(stderr, "Usage: %s interface\n", argv[0]);
      exit(EXIT_FAILURE);
    }

  signal(SIGINT, sigint);

  rsock = create_raw_socket();
  ifidx = get_interface_id(rsock, argv[1]);
  get_interface_addr(rsock, argv[1], src_hwaddr);
  bind_interface(rsock, ifidx);

  printf("Bound to interface #%d (%x:%x:%x:%x:%x:%x)\n", ifidx,
	 src_hwaddr[0], src_hwaddr[1], src_hwaddr[2],
	 src_hwaddr[3], src_hwaddr[4], src_hwaddr[5]);
  printf("--- Sending ARP requests ---\n");


  src_ip = 0x0100a8c0; /* 192.168.0.1 */
  dst_ip = 0xfe00a8c0; /* 192.168.0.254 */
  memset(dst_hwaddr, 0xff, 6);
  craft_eth(&packet.eth_head, 0x0806,
	    src_hwaddr,
	    dst_hwaddr);
  memset(dst_hwaddr, 0x00, 6);
  craft_arp(&packet.arp_head,
	    src_hwaddr, (unsigned char *)&src_ip,
	    dst_hwaddr, (unsigned char *)&dst_ip);

  if (write(rsock, &packet, sizeof(packet)) == -1)
    fprintf(stderr, "write(): Error: %s.\n", strerror(errno));

  close(rsock);
  printf("--- Stop sending ARP requests ---\n");
  return (EXIT_SUCCESS);
}

void sigint(int __attribute__ ((unused)) n)
{
  close(rsock);
  printf("\r--- Stop sending ARP requests ---\n");
  exit(EXIT_SUCCESS);
}
