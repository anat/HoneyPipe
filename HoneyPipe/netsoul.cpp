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
    int i = 0;
    tcp* pTCP = (tcp*)p.getBuffer();

    if (portA && portB)
      {
	if ((pTCP->source == portA && pTCP->dest == portB) ||
	    (pTCP->source == portB && pTCP->dest == portA))
	  return true;
      }
    else
      {
	while (begin[i])
	  if (!strncmp(begin[i], data, strlen(begin[i++])))
	    {
	      portA = pTCP->source;
	      portB = pTCP->dest;
	      write(1, data, p.Size - sizeof(tcp));
	      return true;
	    }
      }
    return false;
}


int Netsoul::sendTargetAToTargetB(Packet & p)
{
    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    std::cout << "\t\tRECEIVED" << std::endl;
    int i = 0;
    while (data[i] != '\n' && i < p.Size - sizeof(tcp))
        i++;
    if (data[i] == '\n')
    {
        data[i] = 0;
    }
    else
    {
        std::cout << "Bizarre pas de \n ou fin du buffer" << std::endl;
    }
    this->ui->out->setPlainText(QString(data));
    return 0;
}


int Netsoul::sendTargetBToTargetA(Packet & p)
{
    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    std::cout << "\t\tRECEIVED" << std::endl;
    int i = 0;
    while (data[i] != '\n' && i < p.Size - sizeof(tcp))
        i++;
    if (data[i] == '\n')
    {
        data[i] = 0;
    }
    else
    {
        std::cout << "Bizarre pas de \n ou fin du buffer" << std::endl;
    }
    this->ui->in->setPlainText(QString(data));
    return 0;
}
