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

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        nbItem(0),
        currentProtocol(NULL),
        state(0)
{
    ui->setupUi(this);

    foreach(QNetworkInterface iter, QNetworkInterface::allInterfaces())
    {
        if(iter.isValid())
            if (!iter.hardwareAddress().isEmpty())
                ui->cbInt->addItem(iter.humanReadableName());
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
    uint8_t dst_hwaddr[6];
    memset(dst_hwaddr, 0xff, 6);
    this->addNewItem("Test ", dst_hwaddr);


    QObject::connect(ui->cbInt, SIGNAL(currentIndexChanged(QString)), this, SLOT(fillIps(QString)));
    QObject::connect(ui->btnScan, SIGNAL(clicked()), this, SLOT(scan()) );
    QObject::connect(ui->pbSetDest, SIGNAL(clicked()), this, SLOT(setDest()));
    QObject::connect(ui->pbSetSource, SIGNAL(clicked()), this, SLOT(setSource()));
    QObject::connect(ui->pbSpoof, SIGNAL(clicked()), this, SLOT(startSpoofing()));
    QObject::connect(ui->pbPlay, SIGNAL(clicked()), this, SLOT(play()));
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::fillIps(QString interface)
{
    ui->cbIp->clear();
    foreach (QHostAddress h, QNetworkInterface::interfaceFromName(interface).allAddresses())
        if (h.protocol() == QAbstractSocket::IPv4Protocol)
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
        Packet p;

        u_int32_t ipsrc, ipdst;
        QHostAddress tmp;
        tmp.setAddress(this->ui->leSourceIP->text());
        ipsrc = htonl(tmp.toIPv4Address());
        tmp.setAddress(this->ui->leRouterIP->text());
        ipdst = htonl(tmp.toIPv4Address());

        while (this->state & Playing)
        {
            // poll each ms
            if (s.Poll(1000))
            {
                s.Read(p, true);
                std::cout << "Packet received : " << p.Size << std::endl;
                ip* pIP = (ip*)p.getBuffer();
                //std::cout << (int)pIP->ip_p << std::endl;

                if (pIP->isTCP())
                {
                    tcp* pTCP = (tcp*)p.getBuffer();
                    std::cout << "Source port : " << pTCP->source << std::endl << "Dest port : " << pTCP->dest << std::endl;
                    if (pIP->ip_dst == ipdst && pIP->ip_src == ipsrc)
                    {
                        // from "client" to "router"
                    }
                    else if (pIP->ip_dst == ipsrc && pIP->ip_src == ipdst)
                    {
                        // from "router" to "client"
                        write(1, ((char *)p.getBuffer()) + sizeof(tcp), p.Size - sizeof(tcp));
                    }

                    if (pTCP->ip_len == p.Size - sizeof(tcp))
                    {
                        std::cout << "Bonne taille" << std::endl;
                    }

                    //write(1, ((char *)p.getBuffer()) + sizeof(tcp), p.Size - sizeof(tcp));

                }
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
                uint8_t * rep = arpr.doRequest(s, currentInterface, htonl(currentIP.ip().toIPv4Address()), htonl(ip));
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
    this->ui->leSourceMAC->setText("macvictim");
    this->ui->leRouterMAC->setText("macrouter");
    //this->ui->leSource->text().toStdString().c_str()
    //this->ui->leDest->text().toStdString().c_str()
    this->statusText->setText("Spoofing ...");
}
