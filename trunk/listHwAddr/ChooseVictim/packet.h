#ifndef PACKET_H
# define PACKET_H
# include "eth.h"
# include "arp.h"
#include <stdlib.h>
#include <iostream>
#include <string.h>

class Packet
{
private:
    void *buffer;
public:
    Packet();
    Packet(ethheader* ethernetHeader);

    template<typename T>
    int append(T* buffer, int size)
    {
        this->buffer = realloc(this->buffer, this->Size + size);
        memcpy(((char *)this->buffer) + this->Size, buffer, size);
        this->Size += size;
        return this->Size;
    }

    void * getBuffer();
    int Size;
};

#endif // PACKET_H
