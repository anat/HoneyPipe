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

    if (isProtocol)
    {
         write(1, data, p.Size - sizeof(tcp));
         char buffer[p.Size];
         memcpy(buffer, data, p.Size);
         buffer[p.Size - 1] = 0;
         QString str((const char *)buffer);
        this->addActivity(str);
    }
    return false;
}

void Netsoul::addActivity(QString & message)
{
    if (this->ui->activity->toPlainText().length() != 0)
        this->ui->activity->setPlainText(this->ui->activity->toPlainText() + message);
    else
        this->ui->activity->setPlainText(this->ui->activity->toPlainText() + message);
}




int Netsoul::sendTargetAToTargetB(Packet & p)
{
    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    std::cout << "\t\tRECEIVED" << std::endl;

    return 0;
}


int Netsoul::sendTargetBToTargetA(Packet & p)
{
    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    std::cout << "\t\tRECEIVED" << std::endl;

    return 0;
}
