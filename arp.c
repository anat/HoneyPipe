#include <string.h>
#define ARP_REQUEST   1               /* ARP request.  */
#define ARP_REPLY     2               /* ARP reply.  */


typedef struct arpheader
{
  unsigned short int ar_hrd;          /* Format of hardware address.  */
  unsigned short int ar_pro;          /* Format of protocol address.  */
  unsigned char ar_hln;               /* Length of hardware address.  */
  unsigned char ar_pln;               /* Length of protocol address.  */
  unsigned short int ar_op;           /* ARP opcode (command).  */

  unsigned char ar_sha[6];   /* Sender hardware address.  */
  unsigned char ar_sip[4];          /* Sender IP address.  */
  unsigned char ar_tha[6];   /* Target hardware address.  */
  unsigned char ar_tip[4];          /* Target IP address.  */

} arph;


int main(int ac, char **av)
{

}


int forgearp(unsigned char *sendermac, unsigned char *senderip, unsigned char *recvmac, unsigned char *recvip)
{
  arph arp;
  arp.ar_hrd = 1;
  arp.ar_pro = 0x0800;
  arp.ar_hln = 6;
  arp.ar_pln = 4; // ipv4
  arp.ar_op = ARP_REPLY;
  memcpy(arp.ar_sha, sendermac, 6);
  memcpy(arp.ar_sip, senderip, 4);
  memcpy(arp.ar_tha, recvmac, 6);
  memcpy(arp.ar_tip, recvip, 4);
}
