#ifndef CHANGEMESSAGE_H
#define CHANGEMESSAGE_H

#include <QDialog>
#include <string>

typedef enum changeMessage{
    A,
    B
   } ChangeMessageState;

namespace Ui {
    class ChangeMessage;
}

class ChangeMessage : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeMessage(ChangeMessageState isFrom, std::string * message, QWidget *parent = 0);
    ~ChangeMessage();
    QString getMessage();
    QString getLogin();
    QString OriginalMessage;
public:
    Ui::ChangeMessage *ui;
};

#endif // CHANGEMESSAGE_H
