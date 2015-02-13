#include "inc/adbsynchelper.h"
#include "QFuture"
#include "QtConcurrent/QtConcurrent"
#include "QFileInfo"
#include "direntry.h"
#include "QDir"


int sync_push(QString lpath, QString rpath);
int sync_pull(QString rPath, QString lPath);

AdbSyncHelper::AdbSyncHelper(QObject *parent) :
    QObject(parent),
    m_progressDialog("Copying files","Abort",0,0),
    m_aborted(false)
{
}


QStringList AdbSyncHelper::generateFileList(QString dirName,QString rPath)
{
    QFileInfo info(dirName);
    QDirIterator dirIterator(dirName,QDir::AllDirs|QDir::NoDotAndDotDot|QDir::Files,
                    QDirIterator::Subdirectories);
    QStringList fileList;

    while (dirIterator.hasNext()) {
        dirIterator.next();

        if(dirIterator.fileInfo().isDir())
            continue;

        QString targetPath = dirIterator.filePath();
        targetPath.replace(dirName,"");
        if(targetPath.startsWith('/')==false)
            targetPath = "/"+targetPath;

        targetPath = rPath+info.fileName()+ targetPath;

        /* source ||| target*/
        fileList.append(dirIterator.filePath() +" ||| " +targetPath);
    }

    return fileList;

}



void AdbSyncHelper::on_startOperation(QString fileName)
{
    m_progressDialog.setLabelText("Copying "+fileName);

}

void AdbSyncHelper::on_finishOperation(QString /*fileName*/)
{
    m_progressDialog.setValue(m_progressDialog.value()+1);

}

void AdbSyncHelper::on_abort()
{
    m_aborted = true;

}
QHash<DirEntry *, QString> AdbSyncHelper::pullFileMap() const
{
    return m_pullFileMap;
}

void AdbSyncHelper::setPullFileMap(const QHash<DirEntry *, QString> &pullFileMap)
{
    m_pullFileMap = pullFileMap;
    m_progressDialog.setMaximum(pullFileMap.size());
}

QStringList AdbSyncHelper::fileList() const
{
    return m_pushfileList;
}

void AdbSyncHelper::setFileList(const QStringList &fileList)
{
    m_pushfileList = fileList;
    m_progressDialog.setMaximum(fileList.size());
}


/* fileNames should contain list of files in format: source ||| destination */
bool AdbSyncHelper::do_adb_push(QStringList fileNames, QString rpath)
{
    AdbSyncHelper helper;
    helper.setFileList(fileNames);
    helper.m_rPath = rpath;

    QFutureWatcher<bool> pushWatcher;

    QObject::connect(&pushWatcher,SIGNAL(finished()),
                     &helper.m_progressDialog,SLOT(close()));
    connect(&helper.m_progressDialog,SIGNAL(canceled()),
            &helper,SLOT(on_abort()));
    connect(&helper,SIGNAL(startSync(QString)),&helper,SLOT(on_startOperation(QString)));
    connect(&helper,SIGNAL(finishSync(QString)),&helper,SLOT(on_finishOperation(QString)));

    pushWatcher.setFuture( QtConcurrent::run(&helper,&AdbSyncHelper::push_thread) );

    helper.m_progressDialog.exec();

    pushWatcher.waitForFinished();

    return pushWatcher.result();
}



/**
 * @brief AdbSyncHelper::push
 * @param lPath List of paths to be pushed to device
 * @param rPath Path on the device
 * @return true if the operation completed. False if it had failures
 */
bool AdbSyncHelper::push(QStringList lPath,QString rPath)
{

    QStringList fileList;

    if(rPath.endsWith('/')==false)
        rPath = rPath+"/";

    QProgressDialog progress("Preparing for copy","Abort",0,0);
    progress.setCancelButton(NULL);
    progress.show();
    qApp->processEvents();

    foreach (QString fileName, lPath) {
        QFileInfo info(fileName);
        progress.setLabelText("Preparing for copy "+info.fileName());
        if(info.isDir())
        {
            QStringList list = AdbSyncHelper::generateFileList(info.absoluteFilePath(),rPath);
            fileList.append(list);
        }
        else
        {
            QString target = QString("%1%2").arg(rPath,info.fileName());
            fileList.append(info.absoluteFilePath()+"|||"+target);
        }
    }

    progress.close();
    qApp->processEvents();

    return do_adb_push(fileList,rPath);

}



bool AdbSyncHelper::push_thread()
{
    if(m_rPath.isEmpty() || m_pushfileList.isEmpty())
        return false;

    QString targetPath;
    QString sourcePath;


    long completedCount  = 0;

    foreach (QString file, m_pushfileList) {


        QStringList list = file.split("|||",QString::SkipEmptyParts);

        sourcePath  = list.at(0).trimmed();
        targetPath = list.at(1).trimmed();

        if(sourcePath.isEmpty())
            continue;

        emit startSync(sourcePath);

        if(targetPath.isEmpty())
            targetPath = m_rPath;

        if(sync_push(sourcePath,targetPath) == 0)
            completedCount++;

        if(m_aborted)
            break;

        emit finishSync(file);

    }

    emit completed();

    return completedCount == m_pushfileList.size();

}

bool AdbSyncHelper::pull_thread()
{
    QHash<DirEntry*,QString> map = m_pullFileMap;
    int count = 0;
    if(map.isEmpty())
        return count;

    foreach (DirEntry *entry, map.keys()) {
        QString targetFile = map.value(entry);
        if(targetFile.isEmpty())
            continue;

        emit startSync(entry->getAbsPath());
        if(entry->isDir())
        {
            if(QDir(targetFile).mkpath("."))
                count++;
        }
        else
         {
            if(sync_pull(entry->getAbsPath(),targetFile))
                count++;
        }
        emit finishSync(entry->getAbsPath());
    }

    emit completed();
    return count == m_pullFileMap.size();

}


bool AdbSyncHelper::do_adb_pull(QHash<DirEntry*,QString> map)
{
    AdbSyncHelper helper;
    helper.setPullFileMap(map);
    helper.m_rPath.clear();

    QFutureWatcher<bool> pullWatcher;

    QObject::connect(&pullWatcher,SIGNAL(finished()),
                     &helper.m_progressDialog,SLOT(close()));
    connect(&helper.m_progressDialog,SIGNAL(canceled()),
            &helper,SLOT(on_abort()));
    connect(&helper,SIGNAL(startSync(QString)),&helper,SLOT(on_startOperation(QString)));
    connect(&helper,SIGNAL(finishSync(QString)),&helper,SLOT(on_finishOperation(QString)));

    pullWatcher.setFuture( QtConcurrent::run(&helper,&AdbSyncHelper::pull_thread) );

    helper.m_progressDialog.exec();

    pullWatcher.waitForFinished();

    return pullWatcher.result();
}


bool AdbSyncHelper::pull(QHash<DirEntry*,QString> map)
{
    return do_adb_pull(map);
}



