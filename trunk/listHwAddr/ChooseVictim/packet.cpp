#include "packet.h"
#include <string.h>
#include <stdlib.h>

Packet::Packet()
{
    this->Size = 0;
    this->buffer = NULL;
}

Packet::Packet(ethheader* ethernetHeader)
{
    this->buffer = malloc(sizeof(ethheader));
    memcpy(this->buffer, ethernetHeader, sizeof(ethheader));
    this->Size = sizeof(ethheader);
}

template<class T>
int Packet::append(T buffer)
{
    this->buffer = realloc(this->buffer, this->Size + sizeof(T));
    memcpy(this->buffer + this->Size, buffer, sizeof(T));
    this->Size += sizeof(T);
    return this->Size;
}

void * Packet::getBuffer()
{
    return this->buffer;
}
