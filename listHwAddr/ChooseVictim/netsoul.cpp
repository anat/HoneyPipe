#include "netsoul.h"
#include "ui_netsoul.h"

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

    return true;
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
