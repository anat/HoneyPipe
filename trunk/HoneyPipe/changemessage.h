#ifndef CHANGEMESSAGE_H
#define CHANGEMESSAGE_H

#include <QDialog>
#include <string>

namespace Ui {
    class ChangeMessage;
}

class ChangeMessage : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeMessage(std::string * message, QWidget *parent = 0);
    ~ChangeMessage();
    QString getMessage();
    QString OriginalMessage;
private:
    Ui::ChangeMessage *ui;
};

#endif // CHANGEMESSAGE_H
