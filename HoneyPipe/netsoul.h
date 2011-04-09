#ifndef NETSOUL_H
#define NETSOUL_H

#include <QMainWindow>
#include "IProtocol.hpp"
#include "packet.h"
#include <string>
#include "changemessage.h"
#define NS_SENDMSG "user_cmd "

namespace Ui {
    class Netsoul;
}

typedef enum NetsoulState{
    NoInterference,
    WaitingForMessage,
    WaitingForTyping
      } NetsoulState;

class Netsoul : public QMainWindow,  private IProtocol
{
    Q_OBJECT
public:
    explicit Netsoul(QWidget *parent = 0);
    ~Netsoul();
    virtual bool isProtocol(Packet & p);
    virtual void sendTargetAToTargetB(Packet & p);
    virtual void sendTargetBToTargetA(Packet & p);
    std::string *isMessage(Packet & p);
    void addActivity(const char * message);
    std::string * getUser(Packet & p);
    std::list<Packet*> Queue;
    bool clearQueue;
    uint32_t NextDelta;
    uint32_t deltaA;
    uint32_t deltaB;
private:
    Ui::Netsoul *ui;
    uint16_t portA;
    uint16_t portB;

    int state;
    ChangeMessage* currentMessage;
public slots:
    void startWaitForMessage();
    void hasMessage();
};

#endif // NETSOUL_H
