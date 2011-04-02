#include "http.h"
#include "ui_http.h"

http::http(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::http)
{
    ui->setupUi(this);
}

http::~http()
{
    delete ui;
}
