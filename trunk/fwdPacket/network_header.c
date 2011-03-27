#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdint.h>
#include "network_header.h"

void craft_arp(arp_t *arp, uint16_t type,
	       uint8_t *srcmac, uint8_t *srcip,
	       uint8_t *dstmac, uint8_t *dstip)
{
  arp->ar_hrd = htons(1);
  arp->ar_pro = htons(0x0800);
  arp->ar_hln = 6;
  arp->ar_pln = 4;
  arp->ar_op = htons(type);
  memcpy(arp->ar_sha, srcmac, 6);
  memcpy(arp->ar_sip, srcip, 4);
  memcpy(arp->ar_tha, dstmac, 6);
  memcpy(arp->ar_tip, dstip, 4);
}

void craft_eth(eth_t *eth, uint16_t type,
	       uint8_t *srcmac, uint8_t *dstmac)
{
  eth->type = htons(type);
  memcpy(eth->ar_tha, dstmac, 6);
  memcpy(eth->ar_sha, srcmac, 6);
}
