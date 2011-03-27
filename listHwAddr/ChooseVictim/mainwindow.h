#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <stdint.h>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    int nbItem;
    void	addNewItem(QString const & ip, uint8_t * mac);

public slots:
    void fillIps(QString interface);
    void scan();
};

#endif // MAINWINDOW_H
