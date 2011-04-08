#ifndef IPROTOCOL_HPP
#define IPROTOCOL_HPP


#include "packet.h"


typedef enum ActionForWriting{
    RoutePacket,
    DropPacket,
    StorePacket
          } PacketState;

class   IProtocol
{
public:
    virtual bool isProtocol(Packet & p) = 0;
    virtual PacketState sendTargetAToTargetB(Packet & p) = 0; // take a packet from A to tamper and return delta
    virtual PacketState sendTargetBToTargetA(Packet & p) = 0; // take a packet from B to tamper and return delta
    virtual ~IProtocol() {}
};

#endif // IPROTOCOL_HPP
