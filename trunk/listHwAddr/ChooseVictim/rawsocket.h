#ifndef RAWSOCKET_H
#define RAWSOCKET_H

#include <stdint.h>
#include "packet.h"


class RAWSocket
{
public:
    RAWSocket();
    ~RAWSocket();
    int Handler;
    bool Create(int index, uint16_t protocol);
    int Read(Packet & p, bool create = false);
    int Write(Packet & p);
    int Poll(int timeout);
private:
    fd_set rfds;
    fd_set wfds;
    void* buffer;

};

#endif // RAWSOCKET_H
