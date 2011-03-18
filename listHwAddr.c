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

void sigint(int n);
int create_raw_socket();
int get_interface_id(int fd, char *name);
void get_interface_addr(int fd, char *name, unsigned char *hwaddr);
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

void get_interface_addr(int fd, char *name, unsigned char *hwaddr)
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

int main(int argc, char **argv)
{
  unsigned char buff[4096];
  unsigned char hwaddr[6];
  int n = 1, cc;
  int ifidx;
  arph *packet;

  if (argc != 2)
    {
      fprintf(stderr, "Usage: %s interface\n", argv[0]);
      exit(EXIT_FAILURE);
    }

  signal(SIGINT, sigint);

  rsock = create_raw_socket();
  ifidx = get_interface_id(rsock, argv[1]);
  get_interface_addr(rsock, argv[1], hwaddr);
  bind_interface(rsock, ifidx);

  printf("Bound to interface #%d (%x:%x:%x:%x:%x:%x)\n", ifidx,
	 hwaddr[0], hwaddr[1], hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
  printf("--- Sending ARP packets ---\n");

  //packet = forge_arp(hwaddr, );
  exit(1);

  if (write(rsock, ping, 98) == -1)
    fprintf(stderr, "write(): Error: %s.\n", strerror(errno));

  close(rsock);
  return (EXIT_SUCCESS);
}

void sigint(int __attribute__ ((unused)) n)
{
  close(rsock);
  printf("\r--- Stop sending ARP packets ---\n");
  exit(EXIT_SUCCESS);
}
