#ifndef IPROTOCOL_HPP
#define IPROTOCOL_HPP


#include "packet.h"
class   IProtocol
{
public:
    virtual bool isProtocol(Packet & p) = 0;
    virtual int sendTargetAToTargetB(Packet & p) = 0; // take a packet from A to tamper and return delta
    virtual int sendTargetBToTargetA(Packet & p) = 0; // take a packet from B to tamper and return delta
    virtual ~IProtocol() {}
};

#endif // IPROTOCOL_HPP
