#include "ui/inc/welcome.h"
#include "ui_welcome.h"

#include "adbhandler.h"
#include "abstractmdichild.h"



void Welcome::refreshStats()
{
    AdbHandler h;
//    QList<AdbDevice> deviceList;

    if(h.isServerRunning())
    {

        ui->startServer->setText("Stop Server");
        setStatus("Server Running");
        ui->serverStatus->setText("Server :Running");

        if(h.isDeviceConnected() == false)
        {
            setStatus("Device Not Connected");
            ui->deviceStatus->setText("Device :Not Connected");
        }
        else
        {
            setStatus("Device Connected");
            showDeviceList(h.listDevices());
            ui->image->setPixmap(QPixmap(":/icons/res/usb_blue.png"));
        }


    }
    else
    {
        setStatus("Server not running");
        ui->serverStatus->setText("Server :Not Running");
        ui->deviceStatus->setText("Device :Not Connected");
        ui->startServer->setText("Start Server");
        ui->image->setPixmap(QPixmap(":/icons/res/splash.png"));
    }

}

void Welcome::showDeviceList(QList<AdbDevice> list)
{
    QString str = QString("Devices: \n");
    foreach (AdbDevice dev, list) {
        str.append(QString("%1 \n").arg(dev.name));
    }
    ui->deviceStatus->setText(str);
}

void Welcome::setStatus(QString status)
{
//    ui->serverStatus->setText(QString("Status: %1").arg(status));
    ui->title->setText(QString("Status: %1").arg(status));
    setWindowTitle(status);
}

Welcome::Welcome(QWidget *parent) :
    AbstractMDIChild(parent),
    ui(new Ui::Welcome)
{
    ui->setupUi(this);
    refreshStats();

}

Welcome::~Welcome()
{
    delete ui;
}

void Welcome::on_refreshDeviceList_clicked()
{
    refreshStats();
}

void Welcome::on_startServer_clicked()
{
    AdbHandler h;
    if(h.isServerRunning() == false)
        h.startAdbServer();
    else
        h.killServer();

    refreshStats();
}
