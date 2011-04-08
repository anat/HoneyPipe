#ifndef HTTP_H
#define HTTP_H

#include <QMainWindow>
#include "IProtocol.hpp"
#include "packet.h"

namespace Ui {
    class http;
}

class http : public QMainWindow,  private IProtocol
{
    Q_OBJECT

public:
    explicit http(QWidget *parent = 0);
    virtual bool isProtocol(Packet & p);
    virtual PacketState sendTargetAToTargetB(Packet & p);
    virtual PacketState sendTargetBToTargetA(Packet & p);
    ~http();

private:
    Ui::http *ui;
    uint16_t portA;
    uint16_t portB;
};

#endif // HTTP_H
