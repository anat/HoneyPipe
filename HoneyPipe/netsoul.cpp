#include "netsoul.h"
#include "ui_netsoul.h"
#include <string>
#include <cstdio>
#include "packet.h"
Netsoul::Netsoul(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::Netsoul),
	portA(0),
	portB(0)
{
    ui->setupUi(this);
}

Netsoul::~Netsoul()
{
    delete ui;
}


bool Netsoul::isProtocol(Packet & p)
{

    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    const char * begin[] = {"salut", "auth_ag", "list_users", "ping", "user_cmd", "state", "exit", NULL};

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

void Netsoul::addActivity(QString & message)
{
    this->ui->activity->setPlainText("-" + message + this->ui->activity->toPlainText());
}




int Netsoul::sendTargetAToTargetB(Packet & p)
{
    char * data = ((char*)p.getBuffer()) + sizeof(tcp);

    char buffer[p.Size - sizeof(tcp) + 1];
    memcpy(buffer, data, p.Size - sizeof(tcp));
    buffer[p.Size - sizeof(tcp)] = 0;
    QString str("Packet from client : \"");
    str += (const char *)buffer;
    str += "\"";
    this->addActivity(str);

    return 0;
}


int Netsoul::sendTargetBToTargetA(Packet & p)
{
    char * data = ((char*)p.getBuffer()) + sizeof(tcp);

    char buffer[p.Size - sizeof(tcp) + 1];
    memcpy(buffer, data, p.Size - sizeof(tcp));
    buffer[p.Size - sizeof(tcp)] = 0;
    QString str("Packet from router : \"");
    str += (const char *)buffer;
    str += "\"";
    this->addActivity(str);

    return 0;
}

char *Netsoul::isMessage(Packet & p)
{
    char *data = ((char*)p.getBuffer()) + sizeof(tcp);
    int i;

    if (!strncmp(NS_SENDMSG, data, strlen(NS_SENDMSG)))
      {
	data += strlen(NS_SENDMSG);
	for (i = 0; data[i]; i++)
	  if (!strncmp("msg ", data+i, 4) && i)
	    if ((i < 9 || strncmp(NS_SENDMSG, data+i-9, 8)))
	      return data+i+4;
      }
    return NULL;
}
