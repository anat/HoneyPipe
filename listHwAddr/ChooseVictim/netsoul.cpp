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
