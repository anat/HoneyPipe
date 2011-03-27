#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <stdint.h>
#include <QtGui/QLabel>
namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QLabel* statusText;
private:
    Ui::MainWindow *ui;
    int nbItem;
    void	addNewItem(QString const & ip, uint8_t * mac);

public slots:
    void fillIps(QString interface);
    void scan();
    void setDest();
    void setSource();
    void startSpoofing();
};

#endif // MAINWINDOW_H
