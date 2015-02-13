#ifndef ADBHANDLER_H
#define ADBHANDLER_H

#include <QObject>
#include <QThread>
#include <QStringList>
#include "fileobject.h"
#include "adblistner.h"
#include "QFileSystemModel"
#include "direntry.h"

extern "C" {
#include "fdevent.h"
#include "adb.h"
}


struct AdbDevice
{
    QString serial;
    QString name;
    bool isAuthorised;
};

class AdbHandler : public QObject
{
    Q_OBJECT
public:
    explicit AdbHandler(QObject *parent = 0);
    QList<AdbDevice> listDevices();
    QList<FileObject> getDirListing(QString dirPath);

    bool isServerRunning();
    bool isDeviceConnected();
    bool waitForDevice();
    void startAdbServer();
    bool killServer();

private:
    void init();
    
signals:
    
public slots:
    void abort();
//    void startSendingFolder(QString dir);


private:
    int  _adbFD;
    AdbListner *_adbListner;
    QThread    *_adbThread;
    transport_type _transportType;
    char * _serial;
    int m_coreCount;

    QString readString(int fd);
    bool device_wait_thread();

    bool m_abortCurrentOp;

    QString m_lastError;

    void setLastError(const QString &lastError);


public:
    bool isRunning()
    {
        if(_adbThread == 0)
            return false;
        return _adbThread->isRunning();
    }
    
    QByteArray getTraceLog(int seconds =1, bool rooted = false);
    int getCpuCount();
    int getSdkVersion();
    bool isAborted() const;
    void AbortCurrentOp(bool isAborted);
    QList<long> getCpuSpeeds();
    long getCpuSpeed(int cpu, bool retry = false);
    QString getPropValue(QString prop);
    long getMaxCpuSpeed(int cpu, bool retry = false);
    QList<long> getCpuMaxSpeeds();

    bool push(QString lPath, QString rPath);    
    bool push(QStringList lPath, QString rPath);

    bool pull(QHash<DirEntry *, QString> map);
    bool pull(DirEntry *entry, QString target);

    bool moveFile(QString source, QString dest);
    bool deleteFile(QString fileName);
    QString lastError() const;

    void rebootDevice(QString options);
    QString executeCommand(QString cmd, QString arg1 = QString(), QString arg2 = QString());
};

#endif // ADBHANDLER_H
