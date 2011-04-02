#include "http.h"
#include "ui_http.h"
#include "packet.h"

http::http(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::http)
{
    ui->setupUi(this);
}

http::~http()
{
    delete ui;
}

bool http::isProtocol(Packet & p)
{

    return true;
}

int http::sendTargetBToTargetA(Packet & p)
{

    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    std::cout << "\t\tRECEIVED" << std::endl;
    write(1, data, 10);
    if (!strncmp("HTTP/1.1 ", data, 9))
    {
        this->ui->pte->setPlainText(data);
    }
    return 0;
}

int http::sendTargetAToTargetB(Packet & p)
{

    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    std::cout << "\t\tRECEIVED" << std::endl;
    write(1, data, 10);
    if (!strncmp("HTTP/1.1 ", data, 9))
    {
        this->ui->pte->setPlainText(data);
    }
    return 0;
}
