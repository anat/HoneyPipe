#ifndef ARPREQUEST_H
#define ARPREQUEST_H

#include <QtNetwork/QNetworkInterface>
#include <stdint.h>
#include <string>
#include "rawsocket.h"

class ARPRequest
{
public:
    ARPRequest();
    uint8_t* doRequest(RAWSocket & s, QNetworkInterface const & interface, int src_ip, int dst_ip, int timeout);
};

#endif // ARPREQUEST_H
