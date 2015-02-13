#include "adbhandler.h"
#include "direntry.h"
#include "QDebug"
#include "errno.h"

#include "QtConcurrent/QtConcurrent"
#include "QProgressDialog"

extern "C" {
#include "file_sync_service.h"
#include "fdevent.h"
#include "adb.h"
#include "adb_client.h"

#ifndef _WIN32
#include "sysdeps.h"
#else
int  adb_close(int  fd);
int  adb_read(int  fd, void* buf, int len);
int sync_readmode(int fd, const char *path, unsigned *mode);
void sync_quit(int fd);
#endif

}
#include "stdint.h"

#undef fprintf

int sync_push(QString lpath, QString rpath);

AdbHandler::AdbHandler(QObject *parent) :
    QObject(parent)
{
    _transportType = kTransportAny;
    _serial = NULL;
    init();

    m_abortCurrentOp = false;
}


QString AdbHandler::getPropValue(QString prop)
{

    QString cmd("shell: getprop ");
    cmd+=prop;

    int fd = adb_connect((const char *)cmd.toLocal8Bit().data());

    if(fd < 0) {
        qDebug() <<"error: " << adb_error();
        return QString();
    }

    QString str = readString(fd);

    adb_close(fd);

    return str;

}

/* getprop ro.build.version.sdk*/
int AdbHandler::getSdkVersion()
{
    QString versionString = getPropValue("ro.build.version.sdk");

    if(versionString.isEmpty())
        return -1;

    return versionString.toInt();
}

QList<AdbDevice> AdbHandler::listDevices()
{

    char *tmp;    
    QStringList ret;
    QList<AdbDevice> deviceList;

    QString buff("host:devices");
    tmp = adb_query((char*)buff.toLocal8Bit().data());
    if(tmp) {        
        ret = QString(tmp).split("\n");
        free(tmp);
    }
    foreach (QString device, ret) {
        AdbDevice a;
        if(device.isEmpty())
            continue;
        QStringList detail = device.split("\t");
        a.name = detail.at(0);
        a.serial = detail.at(1);
        if(a.serial.compare("unauthorized",Qt::CaseInsensitive)==0 ||
               a.serial.compare("offline",Qt::CaseInsensitive)==0 )
            a.isAuthorised = false;
        else
            a.isAuthorised = true;
        deviceList.append(a);
    }
    return deviceList;
}

void AdbHandler::init()
{
    adb_set_transport(_transportType , _serial);
    adb_set_tcp_specifics(DEFAULT_ADB_PORT);
}

void AdbHandler::abort()
{
    m_abortCurrentOp = true;
}



QString AdbHandler::readString(int fd)
{
    QString out;

    char buf[128];
    int len;


    while(fd >= 0) {

        len = adb_read(fd, buf, 128);
        if(len == 0) {
            break;
        }

        if(len < 0) {
            if(errno == EINTR) continue;
            break;
        }
        buf[len] = 0;
        out.append((const char*) buf);
    }

    return out;


}


int AdbHandler::getCpuCount()
{
    QString cmd("shell: cat /sys/devices/system/cpu/possible");

    int fd = adb_connect((const char *)cmd.toLocal8Bit().data());

    if(fd < 0) {
        qDebug() <<"error: " << adb_error();
        return -1;
    }

    QString out = readString(fd);

    adb_close(fd);


    if(out.isEmpty())
        return -1;

    if(out.indexOf('-') < 0)
    {
        return out.toInt();
    }

    int len = out.split("-",QString::SkipEmptyParts).at(1).toInt();

    m_coreCount = len+1;

    return m_coreCount;
}

long AdbHandler::getMaxCpuSpeed(int cpu,bool retry)
{
    QString cmd("shell: cat /sys/devices/system/cpu/cpu%1/cpufreq/scaling_max_freq");

    QString cmd_args = cmd.arg(cpu);

    if(retry)
        cmd_args = cmd_args.replace("scaling_","cpuinfo_");

    int fd = adb_connect((const char *)cmd_args.toLocal8Bit().data());

    if(fd < 0) {
        qDebug() <<"error: " << adb_error();
        return -1;
    }

    QString out = readString(fd);

    adb_close(fd);


    if(out.isEmpty())
        return -1;

    bool ok = 0;
    long freq = out.toLong(&ok);
    if(!ok)
        return -1;
    return freq;

}

long AdbHandler::getCpuSpeed(int cpu,bool retry)
{
    QString cmd("shell: cat /sys/devices/system/cpu/cpu%1/cpufreq/scaling_cur_freq");

    QString cmd_args = cmd.arg(cpu);

    if(retry)
        cmd_args = cmd_args.replace("scaling_","cpuinfo_");

    int fd = adb_connect((const char *)cmd_args.toLocal8Bit().data());

    if(fd < 0) {
        qDebug() <<"error: " << adb_error();
        return -1;
    }

    QString out = readString(fd);

    adb_close(fd);

    if(out.isEmpty())
        return -1;

    bool ok;
    long freq = out.toLong(&ok);
    if(!ok)
        return -1;
    return freq;


}


QList<long> AdbHandler::getCpuMaxSpeeds()
{
    QList<long> ret;
    long freq;
    if(m_coreCount <=0 )
        getCpuCount();

    if(m_coreCount <=0)
        return ret;

    bool valid = false;
    for(int i=0;i<m_coreCount;i++)
    {
        freq = getMaxCpuSpeed(i);        
        if(freq < 0)
            freq = getMaxCpuSpeed(i,true);

        if(freq > 0)
            valid = true;

        ret.append(freq);
    }
    if(valid)
        return ret;

    return QList<long>();
}



QList<long> AdbHandler::getCpuSpeeds()
{
    QList<long> ret;
    long freq;
    if(m_coreCount <=0 )
        getCpuCount();

    if(m_coreCount <=0)
        return ret;

    bool valid = false;
    for(int i=0;i<m_coreCount;i++)
    {
        freq = getCpuSpeed(i);
        if(freq > 0)
            valid = true;
        ret.append(freq);
    }
    if(valid)
        return ret;

    return QList<long>();
}


QByteArray AdbHandler::getTraceLog(int seconds, bool rooted)
{

    int version = getSdkVersion();
    QString atraceCommand;
    QString workQueuTraceOption;
    if(version <=17)
    {
        atraceCommand = QString("atrace -b 32768 -z -t %1 -s").arg(seconds);
        workQueuTraceOption = "-w";
    }
    else
    {
        atraceCommand = QString("atrace -b 32768 -z -t %1 sched").arg(seconds);
        workQueuTraceOption = "workq";
    }

    QString cmd;
    if(rooted)
        cmd = QString("shell: su -c \"%1 %2\" ").arg(atraceCommand).arg(workQueuTraceOption);
    else
        cmd = QString("shell: %1").arg(atraceCommand);


//    QString cmd = atraceCommand.arg(seconds);
    int fd = adb_connect((const char *)cmd.toLocal8Bit().data());

    if(fd < 0) {
        qDebug() <<"error: " << adb_error();
        return QByteArray();
    }

    char buf[4096];
    int len;
    QByteArray out;

//    QThread::sleep(2);

    while(fd >= 0) {

        len = adb_read(fd, buf, 4096);
        if(len == 0) {
            break;
        }

        if(len < 0) {
            if(errno == EINTR) continue;
            break;
        }
        out.append(buf,len);
    }

    adb_close(fd);

    return out;

}

QList<FileObject> AdbHandler::getDirListing(QString dirPath)
{
    QList<FileObject> list;
    syncmsg msg;
    char buf[257];
    int len;
    int fd = adb_connect("sync:");

    if(fd < 0) {
        qDebug() <<"error: " << adb_error();
        return QList<FileObject>();
    }



    len = dirPath.size();//strlen(dirPath);//.length();
//    if(len > 1024) goto fail;

    msg.req.id = ID_LIST;
    msg.req.namelen = htoll(len);

    if(writex(fd, &msg.req, sizeof(msg.req)) ||
       writex(fd, dirPath.toLocal8Bit().data(), len)) {
        adb_close(fd);
        return list;
    }

    for(;;) {
        if(readx(fd, &msg.dent, sizeof(msg.dent))) break;
        if(msg.dent.id == ID_DONE) break;
        if(msg.dent.id != ID_DENT) break;

        len = ltohl(msg.dent.namelen);
        if(len > 256) break;

        if(readx(fd, buf, len)) break;
        buf[len] = 0;

        /* skip prev and current dir*/
        if(buf[0] == '.') {
            if(buf[1] == 0) continue;
            if((buf[1] == '.') && (buf[2] == 0)) continue;
        }

        FileObject fo;
        fo.m_associationType = ltohl(msg.dent.mode);
        fo.m_fileSize  = ltohl(msg.dent.size);
        fo.m_fileName=QString(buf);

        /* we use the file name location as id. thats the only thing unique now*/
        fo.objectID = FILE_OBJECT_INVALID_ID ;

        list.append(fo);
//        free(fo.objectInfo.Filename);
    }


    if(msg.dent.id != ID_DONE)
            adb_close(fd);
    else
    {
        /* quit sync*/
        syncmsg msg;
        msg.req.id = ID_QUIT;
        msg.req.namelen = 0;
        writex(fd, &msg.req, sizeof(msg.req));
    }

    adb_close(fd);

    return list;

}

bool AdbHandler::isServerRunning()
{
    // first query the adb server's version
    int fd = _adb_connect("host:version");
    if(fd >= 0)
    {
        adb_close(fd);
        return true;
    }

    return false;

}

bool AdbHandler::isDeviceConnected()
{
    QList<AdbDevice> list = listDevices();

    if(list.isEmpty()) /* No devices*/
        return false;

    /* check if atleast one of them is authorised*/
    foreach (AdbDevice device, list) {
        if(device.isAuthorised == true)
            return true;
    }

    /* Try to wait for some time*/
    QThread::sleep(2);

    list = listDevices();

    /* check if atleast one of them is authorised*/
    foreach (AdbDevice device, list) {
        if(device.isAuthorised == true)
            return true;
}

    return false;
}

bool AdbHandler::device_wait_thread()
{
    do{

    if(isDeviceConnected())
        return true;
    if(isAborted())
        return false;

    QThread::sleep(2);

    }while(true);
}
QString AdbHandler::lastError() const
{
    return m_lastError;
}

void AdbHandler::setLastError(const QString &lastError)
{
    m_lastError = lastError;
}



bool AdbHandler::isAborted() const
{
    return m_abortCurrentOp;
}

void AdbHandler::AbortCurrentOp(bool abortCurrentOp)
{
    m_abortCurrentOp = abortCurrentOp;
}





bool AdbHandler::waitForDevice()
{
    QProgressDialog progress("Waiting for device...","Abort", 0, 0);
    progress.setWindowModality(Qt::WindowModal);

    QObject::connect(&progress,SIGNAL(canceled()),
                     this,SLOT(abort()));

    progress.show();

    QFutureWatcher<bool> deviceWatcher;

    QObject::connect(&deviceWatcher,SIGNAL(finished()),
                     &progress,SLOT(close()));

    deviceWatcher.setFuture( QtConcurrent::run(this,&AdbHandler::device_wait_thread));


    progress.exec();

    deviceWatcher.waitForFinished();

    return deviceWatcher.result();


}

#include "QMessageBox"
void AdbHandler::startAdbServer()
{
    int i;
    i = launch_server(DEFAULT_ADB_PORT);
    if(i<= -3)
        QMessageBox::critical(NULL,"Error","Cannot start ADB Server");
    else if(i< 0)
        QMessageBox::critical(NULL,"Error","Error starting ADB Server");
}

bool AdbHandler::killServer()
{
    int fd;
    fd = _adb_connect("host:kill");
    if(fd == -1) {
        /* server not running*/
        return false;
    }
    adb_close(fd);
    return true;
}

#include "QFileSystemModel"
#include "QProgressDialog"
#include "adbsynchelper.h"



/* Push from lpath -> rPath on device*/
bool AdbHandler::push(QString lPath,QString rPath)
{
    QStringList list;
    list.append(lPath);
    return AdbSyncHelper::push(list,rPath);
}

bool AdbHandler::push(QStringList lPath,QString rPath)
{
    return AdbSyncHelper::push(lPath,rPath);
}


bool AdbHandler::pull(DirEntry *entry,QString target)
{
    QHash<DirEntry*,QString> map;
    map.insert(entry,target);
    return AdbSyncHelper::pull(map);
}


bool AdbHandler::pull(QHash<DirEntry*,QString> map)
{
    return AdbSyncHelper::pull(map);
}

QString AdbHandler::executeCommand(QString cmd,QString arg1,QString arg2)
{
    QString commandLine("shell: %1 %2 %3 ");
    commandLine = commandLine.arg(cmd,arg1,arg2);

    int fd = adb_connect((const char *)commandLine.toLocal8Bit().data());

    if(fd < 0) {
        qDebug() <<"error: " << adb_error();
        return false;
    }

    QString str = readString(fd);

    adb_close(fd);

    return str;

}

bool AdbHandler::moveFile(QString source,QString dest)
{
    if(source.isEmpty() || dest.isEmpty())
        return false;

    return executeCommand("mv",source,dest).isEmpty();
}

bool AdbHandler::deleteFile(QString fileName)
{
    if(fileName.isEmpty())
        return false;

    return executeCommand("rm -rf",fileName).isEmpty(); /* no messages on success*/
}

void AdbHandler::rebootDevice(QString options)
{

    QString cmd("reboot:%1");
    cmd = cmd.arg(options);

    int fd = adb_connect((const char *)cmd.toLocal8Bit().data());

    if(fd < 0) {
        qDebug() <<"error: " << adb_error();
        return ;
    }

    QString str = readString(fd);
    adb_close(fd);

    if(str.isEmpty() == false)
    {
        qDebug()<<str;
        setLastError(str);
    }

    return; /* no messages on success*/

}







