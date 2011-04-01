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
    int sendTargetAToTargetB();
    int sendTargetBToTargetA();
private:
    Ui::Netsoul *ui;
};

#endif // NETSOUL_H
