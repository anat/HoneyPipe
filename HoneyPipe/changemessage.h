#ifndef CHANGEMESSAGE_H
#define CHANGEMESSAGE_H

#include <QDialog>

namespace Ui {
    class ChangeMessage;
}

class ChangeMessage : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeMessage(QWidget *parent = 0);
    ~ChangeMessage();

private:
    Ui::ChangeMessage *ui;
};

#endif // CHANGEMESSAGE_H
