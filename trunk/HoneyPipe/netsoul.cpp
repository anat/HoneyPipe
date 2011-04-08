#include "netsoul.h"
#include "ui_netsoul.h"
#include <QtGui/QInputDialog>
#include <QtCore/QDate>
#include <string>
#include <cstdio>
#include <sstream>
#include "packet.h"

Netsoul::Netsoul(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::Netsoul),
	portA(0),
        portB(0),
        deltaA(0),
        deltaB(0),
        state(0)
{
    ui->setupUi(this);
    QObject::connect(this->ui->changeMessage, SIGNAL(clicked()), this, SLOT(startWaitForMessage()));
}

Netsoul::~Netsoul()
{
    delete ui;
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




PacketState Netsoul::sendTargetAToTargetB(Packet & p)
{
    char * data = ((char*)p.getBuffer()) + sizeof(tcp);
    std::string * msg;


    tcp* pTCP = static_cast<tcp*>(p.getBuffer());
    QString message("A>>> size = " + QString::number(p.Size - sizeof(tcp))
    + "\tisACK = " + QString::number(pTCP->ack) + "\tseq = "
    + QString::number(htonl(pTCP->seq)) + "\tack = "
    + QString::number(htonl(pTCP->ack_seq)));
    this->addActivity(message.toStdString().c_str());

    bool nextDelta = 0;
    if ((msg = this->isMessage(p)))
    {
        if (this->state & WaitingForMessage)
        {
            QString res = QInputDialog::getText(this, "Change Message", "Modify this message", QLineEdit::Normal, QString(msg->c_str()));
            this->state -= WaitingForMessage;
            this->ui->changeMessage->setText("Change next message");
        }
        QString message("A>>> Message from " + QString(this->getUser(p)->c_str()) + " -" + QString(msg->c_str()) + "-");
        this->addActivity(message.toStdString().c_str());

        if (*msg == "test")
        {
            unsigned char buffer[p.Size - sizeof(tcp) + 1];
            memcpy(buffer, data, p.Size - sizeof(tcp)); // without \r\n
            buffer[p.Size - sizeof(tcp)] = 0;
            QString  tmp((const char *)buffer);
            QString res = tmp.replace(QString(msg->c_str()), "CACA2");
            this->addActivity(res.toStdString().c_str());
            this->addActivity("NEW MESSAGE :");
            p.reduce(p.Size - sizeof(tcp));
            p.append(res.toStdString().c_str(), res.length());
            pTCP->ip_len = htons(htons(pTCP->ip_len) + 1);
            nextDelta = 1;
            //deltaA += 1;
        }
        QString str("A>>> Unrecognized Packet : \"");
    }
    else
    {
        char buffer[p.Size - sizeof(tcp) - 1];
        memcpy(buffer, data, p.Size - sizeof(tcp) - 2); // without \r\n
        buffer[p.Size - sizeof(tcp) - 2] = 0;
        QString str("A>>> Unrecognized Packet : \"");
        str += (const char *)buffer;
        str += "\"";
        //this->addActivity(str.toStdString().c_str());
    }
    pTCP->seq = htonl(htonl(pTCP->seq) + deltaA);
    pTCP->ack_seq = htonl(htonl(pTCP->ack_seq) + deltaB);
    if (nextDelta)
        deltaA += nextDelta;
    p.computeChecksum();
    return RoutePacket;
}


PacketState Netsoul::sendTargetBToTargetA(Packet & p)
{
    std::string * msg;
    tcp* pTCP = static_cast<tcp*>(p.getBuffer());
    QString message("<<<B size = " + QString::number(p.Size - sizeof(tcp))
    + "\tisACK = " + QString::number(pTCP->ack) + "\tseq = "
    + QString::number(htonl(pTCP->seq)) + "\tack = "
    + QString::number(htonl(pTCP->ack_seq)));
    this->addActivity(message.toStdString().c_str());



    if ((msg = this->isMessage(p)))
    {
        if (this->state & WaitingForMessage)
        {
            QString res = QInputDialog::getText(this, "Change Message", "Modify this message", QLineEdit::Normal, QString(msg->c_str()));
            this->state -= WaitingForMessage;
            this->ui->changeMessage->setText("Change next message");
        }
        QString message("<<<B Message -" + QString(msg->c_str()) + "-");
        this->addActivity(message.toStdString().c_str());
    }
    else
    {

        char * data = ((char*)p.getBuffer()) + sizeof(tcp);
        char buffer[p.Size - sizeof(tcp) - 1];
        memcpy(buffer, data, p.Size - sizeof(tcp) - 2); // without \r\n
        buffer[p.Size - sizeof(tcp) - 2] = 0;
        QString str("<<<B Unrecognized Packet : \"");
        str += (const char *)buffer;
        str += "\"";
        //this->addActivity(str.toStdString().c_str());
    }
    pTCP->seq = htonl(htonl(pTCP->seq) + deltaB);
    pTCP->ack_seq = htonl(htonl(pTCP->ack_seq) - deltaA);
    p.computeChecksum();
    return RoutePacket;
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
                    //return new std::string(data+i+4);
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


void Netsoul::startWaitForMessage()
{
    // fait foirer les paquets ensuite faut faire ca de facon asynchrone
    //this->state |= WaitingForMessage;
    //this->ui->changeMessage->setText("Stop wait for tamper");
}

