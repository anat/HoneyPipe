#ifndef ARP_H
# define ARP_H

# define ARP_REQUEST   1        /* ARP request.  */
# define ARP_REPLY     2        /* ARP reply.  */

# include <stdint.h>

struct arpheader
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

};


#endif // ARP_H
