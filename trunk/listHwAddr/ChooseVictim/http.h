#ifndef HTTP_H
#define HTTP_H

#include <QMainWindow>
#include "IProtocol.hpp"
#include "packet.h"

namespace Ui {
    class http;
}

class http : public QMainWindow,  private IProtocol
{
    Q_OBJECT

public:
    explicit http(QWidget *parent = 0);
    virtual bool isProtocol(Packet & p);
    int sendTargetBToTargetA(Packet & p);
    ~http();

private:
    Ui::http *ui;
};

#endif // HTTP_H
