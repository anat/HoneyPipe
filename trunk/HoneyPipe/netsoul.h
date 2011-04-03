#ifndef NETSOUL_H
#define NETSOUL_H

#include <QMainWindow>
#include "IProtocol.hpp"
#include "packet.h"

namespace Ui {
    class Netsoul;
}

class Netsoul : public QMainWindow,  private IProtocol
{
    Q_OBJECT

public:
    explicit Netsoul(QWidget *parent = 0);
    ~Netsoul();
    virtual bool isProtocol(Packet & p);
    virtual int sendTargetAToTargetB(Packet & p);
    virtual int sendTargetBToTargetA(Packet & p);
    void addActivity(QString & message);
private:
    Ui::Netsoul *ui;
    uint16_t portA;
    uint16_t portB;
};

#endif // NETSOUL_H
