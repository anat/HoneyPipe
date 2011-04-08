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
    virtual PacketState sendTargetAToTargetB(Packet & p);
    virtual PacketState sendTargetBToTargetA(Packet & p);
    std::string *isMessage(Packet & p);
    void addActivity(const char * message);
    std::string * getUser(Packet & p);
private:
    Ui::Netsoul *ui;
    uint16_t portA;
    uint16_t portB;
    uint32_t deltaA;
    uint32_t deltaB;
    int state;
    Packet waitingPacket;
public slots:
    void startWaitForMessage();
};

#endif // NETSOUL_H
