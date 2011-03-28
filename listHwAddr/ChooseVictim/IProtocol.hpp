#ifndef IPROTOCOL_HPP
#define IPROTOCOL_HPP


#include "packet.h"
class   IProtocol
{
public:
    virtual bool isProtocol(Packet & packet) = 0;
    virtual ~IProtocol() {}
};

#endif // IPROTOCOL_HPP
