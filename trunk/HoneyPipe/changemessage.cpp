#include "changemessage.h"
#include "ui_changemessage.h"

ChangeMessage::ChangeMessage(std::string * message, QWidget *parent) :
    QDialog(parent),
    OriginalMessage(QString(message->c_str())),
    ui(new Ui::ChangeMessage)
{
    ui->setupUi(this);
    ui->message->setPlainText(QString(message->c_str()));
    if (message->length() == 0)
        QObject::connect(ui->btnOk, SIGNAL(clicked()), this->parent(), SLOT(sendNewMessage()));
    else
        QObject::connect(ui->btnOk, SIGNAL(clicked()), this->parent(), SLOT(hasMessage()));
    ui->message->setFocus();
    ui->message->moveCursor(QTextCursor::End);
}

ChangeMessage::~ChangeMessage()
{
    delete ui;
}

QString ChangeMessage::getMessage()
{
    return *(new QString(this->ui->message->toPlainText()));
}
