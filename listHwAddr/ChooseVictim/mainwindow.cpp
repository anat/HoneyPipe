#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui/QMessageBox>
#include <QtNetwork/QNetworkInterface>
#include "arprequest.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    foreach(QNetworkInterface iter, QNetworkInterface::allInterfaces())
    {
        if(iter.isValid())
            if (!iter.hardwareAddress().isEmpty())
                ui->cbInt->addItem(iter.humanReadableName());
    }


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

        int networkAddr = currentIP.ip().toIPv4Address() & currentIP.netmask().toIPv4Address();
        QNetworkAddressEntry qae;
        qae.setIp(QHostAddress(networkAddr));
        QMessageBox::information(this,"test",qae.ip().toString());
        //int dst_ip = 0xfe00a8c0;
        int dst_ip = 0xf000a8c0;
        uint8_t* rep = arpr.doRequest(currentInterface, currentIP.ip().toIPv4Address(), dst_ip);
        if (rep == NULL)
            std::cout << "failed !!!" << std::endl;
        else
            printf("Found MAC : (%x:%x:%x:%x:%x:%x)\n",
                   rep[0], rep[1], rep[2],
                   rep[3], rep[4], rep[5]);

    }

}
