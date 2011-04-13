#ifndef MITMINFO_H
#define MITMINFO_H

#include <stdint.h>

class MITMInfo
{
    public:
    uint32_t ipA;
    uint32_t ipB;
    uint32_t nssIp;
    uint32_t myip;
    uint8_t macA[6];
    uint8_t macB[6];
    uint8_t mymac[6];
};

#endif // MITMINFO_H
