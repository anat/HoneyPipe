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

void * Packet::getBuffer()
{
    return this->buffer;
}
