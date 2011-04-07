#ifndef NETSOUL_H
#define NETSOUL_H

#include <QMainWindow>
#include "IProtocol.hpp"
#include "packet.h"
#include <string>
#define NS_SENDMSG "user_cmd "

namespace Ui {
    class Netsoul;
}

typedef enum NetsoulState{
    WaitingForMessage = 1,
          } NetsoulState;

class Netsoul : public QMainWindow,  private IProtocol
{
    Q_OBJECT

public:
    explicit Netsoul(QWidget *parent = 0);
    ~Netsoul();
    virtual bool isProtocol(Packet & p);
    virtual int sendTargetAToTargetB(Packet & p);
    virtual int sendTargetBToTargetA(Packet & p);
    std::string *isMessage(Packet & p);
    void addActivity(const char * message);
private:
    Ui::Netsoul *ui;
    uint16_t portA;
    uint16_t portB;
    uint32_t seq;
    uint32_t ack;
    int state;
public slots:
    void startWaitForMessage();
};

#endif // NETSOUL_H
