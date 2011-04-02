#ifndef HTTP_H
#define HTTP_H

#include <QMainWindow>

namespace Ui {
    class http;
}

class http : public QMainWindow
{
    Q_OBJECT

public:
    explicit http(QWidget *parent = 0);
    ~http();

private:
    Ui::http *ui;
};

#endif // HTTP_H
