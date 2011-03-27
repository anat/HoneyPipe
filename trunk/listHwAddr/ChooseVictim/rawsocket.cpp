#include "rawsocket.h"
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

RAWSocket::RAWSocket()
{

}


bool RAWSocket::Create(int index, uint16_t protocol)
{
    std::cout << "Creating socket" << std::endl;
    if ((this->Handler = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
    {
        std::cout << "Couldn't create socket !" << std::endl;
        return false;
    }

    struct sockaddr_ll sll;

    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(protocol);
    sll.sll_ifindex = index;
    std::cout << "Binding" << std::endl;
    if (bind(this->Handler, (struct sockaddr *) &sll, sizeof(struct sockaddr_ll)) == -1)
    {
        std::cout << "Couldn't bind !" << std::endl;
        return false;
    }

    return true;
}

int RAWSocket::Read(Packet & p)
{
    return recv(this->Handler, p.getBuffer(), p.Size, 0);
}

int RAWSocket::Write(Packet & p, bool create)
{
    return send(this->Handler, p.getBuffer(), p.Size, 0);
}

int RAWSocket::Poll(int timeout)
{
    FD_ZERO(&this->rfds);
    FD_SET(this->Handler, &(this->rfds));

    FD_ZERO(&this->wfds);
    FD_SET(this->Handler, &(this->wfds));

    std::cout << "HELLO" << std::endl;
    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = timeout;
    int result;
    if ((result = select(this->Handler + 1, &(this->rfds), NULL/*&(this->wfds)*/, NULL, (timeout == 0 ? NULL : &t))) == -1)
        std::cerr << "Select ERROR" << std::endl;
    std::cout << "poll : " << result << std::endl;
    return FD_ISSET(this->Handler, &(this->rfds));
}


RAWSocket::~RAWSocket()
{
 close(this->Handler);
}
