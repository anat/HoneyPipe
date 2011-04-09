#include "changemessage.h"
#include "ui_changemessage.h"

ChangeMessage::ChangeMessage(std::string * message, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeMessage),
    OriginalMessage(QString(message->c_str()))
{
    ui->setupUi(this);
    ui->message->setPlainText(QString(message->c_str()));
    QObject::connect(ui->btnOk, SIGNAL(clicked()), this->parent(), SLOT(hasMessage()));
}

ChangeMessage::~ChangeMessage()
{
    delete ui;
}

QString ChangeMessage::getMessage()
{
    return *(new QString(this->ui->message->toPlainText()));
}
