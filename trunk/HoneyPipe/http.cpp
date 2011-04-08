#include "http.h"
#include "ui_http.h"
#include "packet.h"

http::http(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::http),
    portA(0),
    portB(0)
{
    ui->setupUi(this);
}

http::~http()
{
    delete ui;
}

bool http::isProtocol(Packet & p)
{
    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    const char * begin[] = {"POST", "GET", "PUT", "DELETE", "OPTIONS", "HEAD", NULL};

    tcp* pTCP = (tcp*)p.getBuffer();
    bool isProtocol = false;
    if (portA && portB)
    {
        if ((pTCP->source == portA && pTCP->dest == portB) ||
            (pTCP->source == portB && pTCP->dest == portA))
            isProtocol = true;
    }
    else
    {
        int i = 0;
        while (begin[i])
        {
            if (!strncmp(begin[i], data, strlen(begin[i])))
            {
                portA = pTCP->source;
                portB = pTCP->dest;
                isProtocol = true;
            }
            i++;
        }
    }
    return isProtocol;
}

PacketState http::sendTargetBToTargetA(Packet & p)
{

    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    std::cout << "\t\tRECEIVED" << std::endl;
    write(1, data, 10);
    if (!strncmp("HTTP/1.1 ", data, 9))
    {
        this->ui->pte->setPlainText(data);
    }
    return RoutePacket;
}

PacketState http::sendTargetAToTargetB(Packet & p)
{

    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    std::cout << "\t\tRECEIVED" << std::endl;
    write(1, data, 10);
    if (!strncmp("HTTP/1.1 ", data, 9))
    {
        this->ui->pte->setPlainText(data);
    }
    return RoutePacket;
}
