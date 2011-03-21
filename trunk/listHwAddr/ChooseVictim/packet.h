#ifndef PACKET_H
# define PACKET_H
# include "eth.h"


class Packet
{
private:
    void *buffer;
public:
    Packet();
    Packet(ethheader* ethernetHeader);

template<typename T>
    int append(T buffer);
    void * getBuffer();
    int Size;
};

#endif // PACKET_H
