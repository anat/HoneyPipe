#include "netsoul.h"
#include "ui_netsoul.h"
#include <string>
#include "packet.h"
Netsoul::Netsoul(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Netsoul)
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
    while (begin[i])
        if (strncmp(begin[i], data, strlen(begin[i++])))
            return true;
    return false;
}

// take a packet from A to tamper and return delta
int Netsoul::sendTargetAToTargetB()
{
    return 0;
}

// take a packet from B to tamper and return delta
int Netsoul::sendTargetBToTargetA()
{
    return 0;
}
