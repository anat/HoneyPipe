#ifndef NETWORK_HEADER_H_
# define NETWORK_HEADER_H_

# define ARP_REQUEST   1	/* ARP request.  */
# define ARP_REPLY     2	/* ARP reply.  */

typedef struct ethheader
{
  uint8_t  ar_tha[6];
  uint8_t  ar_sha[6];
  uint16_t type;
} eth_t;

typedef struct arpheader
{
  uint16_t ar_hrd;	/* Format of hardware address.  */
  uint16_t ar_pro;	/* Format of protocol address.  */
  uint8_t  ar_hln;     	/* Length of hardware address.  */
  uint8_t  ar_pln;     	/* Length of protocol address.  */
  uint16_t ar_op;	/* ARP opcode (command).  */

  uint8_t  ar_sha[6];	/* Sender hardware address.  */
  uint8_t  ar_sip[4];	/* Sender IP address.  */
  uint8_t  ar_tha[6];	/* Target hardware address.  */
  uint8_t  ar_tip[4];	/* Target IP address.  */

} arp_t;

void craft_arp(arp_t *arp,
	       uint8_t *srcmac, uint8_t *srcip,
	       uint8_t *dstmac, uint8_t *dstip);
void craft_eth(eth_t *eth, uint16_t type,
	       uint8_t *srcmac, uint8_t *dstmac);

#endif
