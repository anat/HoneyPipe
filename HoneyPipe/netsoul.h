#ifndef NETSOUL_H
#define NETSOUL_H

#include <QMainWindow>
#include "IProtocol.hpp"
#include "packet.h"
#include <string>
#include "changemessage.h"
#define NS_SENDMSG "user_cmd "
#include "rawsocket.h"
#include "mitminfo.h"

namespace Ui {
    class Netsoul;
}

typedef enum NetsoulState{
    NoInterference,
    WaitingForMessageA,
    WaitingForTypingA,
    DropNextMessageA,
    WaitingForMessageB,
    WaitingForTypingB,
    DropNextMessageB
      } NetsoulState;

class Netsoul : public QMainWindow,  private IProtocol
{
    Q_OBJECT
public:
    explicit Netsoul(MITMInfo & info, RAWSocket & socket, QWidget *parent = 0);
    ~Netsoul();
    virtual bool isProtocol(Packet & p);
    virtual void sendTargetAToTargetB(Packet & p);
    virtual void sendTargetBToTargetA(Packet & p);
    std::string *isMessage(Packet & p);
    void addActivityA(const char * message);
    void addActivityB(const char * message);
    std::string * getUser(Packet & p);
    std::list<Packet*> Queue;
    int clearQueue;
    uint32_t NextDelta;
    uint32_t deltaA;
    uint32_t deltaB;
private:
    Ui::Netsoul *ui;
    uint16_t portA;
    uint16_t portB;
    uint32_t currentSeqA;
    uint32_t currentSeqB;
    int state;
    ChangeMessage* currentMessageA;
    ChangeMessage* currentNewMessageA;
    ChangeMessage* currentMessageB;
    ChangeMessage* currentNewMessageB;
    RAWSocket socket;
    MITMInfo info;
public slots:
    void hasMessageA();
    void hasMessageB();
    void sendNewMessageA();
    void startWaitForMessageA();
    void showNewMessageA();
    void dropMessageA();
    void startWaitForMessageB();
    void showNewMessageB();
    void dropMessageB();
};

#endif // NETSOUL_H
