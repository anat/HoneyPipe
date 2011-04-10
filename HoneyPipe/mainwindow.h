#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <stdint.h>
#include <QtGui/QLabel>
#include "rawsocket.h"
#include "mitminfo.h"

namespace Ui {
    class MainWindow;
}

typedef enum state{
    Scanning = 1,
    Spoofing = 2,
    Playing = 4
          } HoneyPipeState;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QLabel* statusText;
    uint32_t deltaBToA;
    uint32_t deltaAToB;
private:
    Ui::MainWindow *ui;
    int nbItem;
    QMainWindow * currentProtocol;
    int state;
    int currentHWIndex;
    void	addNewItem(QString const & ip, uint8_t * mac);
    void newPacket(RAWSocket & s, Packet * p, bool isFromTarget, uint8_t* dstMac);
    MITMInfo info;
public slots:
    void fillIps(QString interface);
    void scan();
    void setDest();
    void setSource();
    void startSpoofing();
    void play();
};

#endif // MAINWINDOW_H
