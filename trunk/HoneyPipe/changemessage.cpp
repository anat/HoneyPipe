#include "changemessage.h"
#include "ui_changemessage.h"

ChangeMessage::ChangeMessage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeMessage)
{
    ui->setupUi(this);
}

ChangeMessage::~ChangeMessage()
{
    delete ui;
}
