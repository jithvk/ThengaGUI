#ifndef WELCOME_H
#define WELCOME_H

#include <QWidget>
//#include "adbhandler.h"
#include "abstractmdichild.h"
struct AdbDevice;


namespace Ui {
class Welcome;
}

class Welcome : public AbstractMDIChild
{
    Q_OBJECT
private:
    void refreshStats();
    void showDeviceList(QList<AdbDevice> list);
    void setStatus(QString status);
public:
    explicit Welcome(QWidget *parent = 0);
    ~Welcome();
    
private slots:
    void on_refreshDeviceList_clicked();

    void on_startServer_clicked();

private:
    Ui::Welcome *ui;
};

#endif // WELCOME_H
