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



    /* FOR UI DEBUG
uint8_t dst_hwaddr[6];
    memset(dst_hwaddr, 0xff, 6);
    this->addNewItem("Test ", dst_hwaddr);
*/

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
        this->ui->leDest->setText(this->ui->twMain->selectedItems().first()->text());
}

void MainWindow::setSource()
{
    if (this->ui->twMain->selectedItems().count() == 0 || this->ui->twMain->selectedItems().count() > 1)
        QMessageBox::information(this, "Error", "You must select 1 item");
    else
        this->ui->leSource->setText(this->ui->twMain->selectedItems().first()->text());
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
        while (this->state & Playing)
        {
            s.Read(p, true);
            if (static_cast<Netsoul*>(this->currentProtocol)->isProtocol(p))
            {;
            }
            s.Write(p);
        }
    }

}

void MainWindow::scan()
{
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
        while ((++i & ~netmask) && (~i & ~netmask))
        {
            ip++;
            current.setIp(QHostAddress(ip));
            this->statusText->setText("Scan is running - " + current.ip().toString());
            uint8_t * rep = arpr.doRequest(s, currentInterface, htonl(currentIP.ip().toIPv4Address()), htonl(ip));
            if (rep != NULL)
                this->addNewItem(current.ip().toString(), rep);
            QCoreApplication::processEvents();
        }
        this->statusText->setText("Scan done");

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
    this->ui->leSource->setText("macvictim");
    this->ui->leDest->setText("macrouter");
    //this->ui->leSource->text().toStdString().c_str()
    //this->ui->leDest->text().toStdString().c_str()
    this->statusText->setText("Spoofing ...");
}
