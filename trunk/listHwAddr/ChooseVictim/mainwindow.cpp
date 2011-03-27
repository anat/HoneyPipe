#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui/QMessageBox>
#include <QtNetwork/QNetworkInterface>
#include "arprequest.h"
#include <iostream>
#include <QtGui/QTableWidgetItem>
#include <QtGui/QStandardItemModel>
#include <arpa/inet.h>

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow),
        nbItem(0)
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
    QObject::connect(ui->cbInt, SIGNAL(currentIndexChanged(QString)), this, SLOT(fillIps(QString)));
    QObject::connect(ui->btnScan, SIGNAL(clicked()), this, SLOT(scan()) );
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


void MainWindow::scan()
{
    ARPRequest arpr;
    if (ui->cbIp->count() != 0)
    {
        // get interface ip and mask
        QNetworkInterface currentInterface = QNetworkInterface::interfaceFromName(ui->cbInt->currentText());
        QNetworkAddressEntry currentIP;
        foreach (QNetworkAddressEntry addr, currentInterface.addressEntries())
            if (addr.ip().toString() == ui->cbIp->currentText())
                currentIP = addr;

        unsigned int networkAddr = currentIP.ip().toIPv4Address() & currentIP.netmask().toIPv4Address();
        QNetworkAddressEntry qae;
        qae.setIp(QHostAddress(networkAddr));
        QMessageBox::information(this,"test",QString::number(currentIP.netmask().toIPv4Address()));
        //int dst_ip = 0xfe00a8c0;
        //int dst_ip = 0xf000a8c0;

        unsigned int ip = networkAddr;
        unsigned int netmask = currentIP.netmask().toIPv4Address();
        QNetworkAddressEntry current;
        uint32_t i = 0;
        while ((++i & ~netmask) && (~i & ~netmask))
          {
            ip++;

        //std::cout << htonl(ip) << std::endl;
            current.setIp(QHostAddress(ip));
            std::cout << "Test " << current.ip().toString().toStdString() << std::endl;
            uint8_t * rep = arpr.doRequest(currentInterface, currentIP.ip().toIPv4Address(), htonl(ip));
            if (rep != NULL)
            {
                printf("Found MAC : (%x:%x:%x:%x:%x:%x)\n",
                    rep[0], rep[1], rep[2],
                    rep[3], rep[4], rep[5]);
                this->addNewItem(current.ip().toString(), rep);
            }
            QCoreApplication::processEvents();
        }


        //this->ui->tableWidget->
      //      std::cout << "failed !!!" << std::endl;
       // else


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
        i->setFlags(Qt::ItemIsEditable);
        QString macAddr;
        macAddr += QString::number(mac[0], 16) + ":" + QString::number(mac[1], 16) + ":" + QString::number(mac[2], 16) + ":" +
                   QString::number(mac[3], 16) + ":" +  QString::number(mac[4], 16) + ":" +  QString::number(mac[5], 16);
        i->setText(macAddr);
        i->setToolTip(macAddr);

        this->ui->twMain->setItem(this->nbItem, 1, i);

        this->nbItem++;
}
