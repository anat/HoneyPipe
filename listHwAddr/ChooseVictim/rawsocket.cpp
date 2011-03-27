#include "rawsocket.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

RAWSocket::RAWSocket()
{

}


bool RAWSocket::Create(int index)
{
    std::cout << "Creating socket" << std::endl;
    if ((this->Handler = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
    {
        std::cout << "Couldn't create socket !" << std::endl;
        return false;
    }

    struct sockaddr_ll sll;

    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_ifindex = index;
    std::cout << "Binding" << std::endl;
    if (bind(this->Handler, (struct sockaddr *) &sll, sizeof(struct sockaddr_ll)) == -1)
    {
        std::cout << "Couldn't bind !" << std::endl;
        return false;
    }
    return true;
}

RAWSocket::~RAWSocket()
{
 close(this->Handler);
}
