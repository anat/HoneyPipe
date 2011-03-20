#ifndef ETH_H
#define ETH_H

struct ethheader
{
  uint8_t  ar_tha[6];
  uint8_t  ar_sha[6];
  uint16_t type;
};

#endif // ETH_H
