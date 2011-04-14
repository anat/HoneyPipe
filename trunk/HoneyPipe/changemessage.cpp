#include "changemessage.h"
#include "ui_changemessage.h"

ChangeMessage::ChangeMessage(ChangeMessageState isFrom, std::string * message, QWidget *parent) :
    QDialog(parent),
    OriginalMessage(QString(message->c_str())),
    ui(new Ui::ChangeMessage)
{
    ui->setupUi(this);
    ui->message->setPlainText(QString(message->c_str()).replace("%20", " "));
    if (isFrom == A)
    {
        if (message->length() == 0)
            QObject::connect(ui->btnOk, SIGNAL(clicked()), this->parent(), SLOT(sendNewMessageA()));
        else
        {
            QObject::connect(ui->btnOk, SIGNAL(clicked()), this->parent(), SLOT(hasMessageA()));
            this->ui->login->setVisible(false);
            this->ui->lblLogin->setVisible(false);
        }
    }
    else if (isFrom == B)
    {
        if (message->length() == 0)
            QObject::connect(ui->btnOk, SIGNAL(clicked()), this->parent(), SLOT(sendNewMessageB()));
        else
        {
            QObject::connect(ui->btnOk, SIGNAL(clicked()), this->parent(), SLOT(hasMessageB()));
            this->ui->login->setVisible(false);
            this->ui->lblLogin->setVisible(false);
        }
    }
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


QString ChangeMessage::getLogin()
{
    return *(new QString(this->ui->login->text()));
}
