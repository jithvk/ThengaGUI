#ifndef ADBSYNCHELPER_H
#define ADBSYNCHELPER_H

#include <QObject>
#include "QProgressDialog"
#include "direntry.h"

class AdbSyncHelper : public QObject
{
    Q_OBJECT
public:
    explicit AdbSyncHelper(QObject *parent = 0);

public:

    static bool push(QStringList lPath, QString rPath);
    static QStringList generateFileList(QString dirName, QString rPath);

    static bool pull(QHash<DirEntry *, QString> map);

signals:
    void startSync(QString fileName);
    void finishSync(QString fileName);
    void completed();



public slots:
    void on_startOperation(QString fileName);
    void on_finishOperation(QString fileName);
    void on_abort();

private:
    QStringList m_pushfileList;
    QHash<DirEntry *, QString> m_pullFileMap;
    QProgressDialog m_progressDialog;
    QString m_rPath;
    bool m_aborted;


    /* Internal helper functions */
private:
    static bool do_adb_pull(QHash<DirEntry *, QString> map);
    static bool do_adb_push(QStringList fileNames,QString rpath);


    QStringList fileList() const;
    void setFileList(const QStringList &fileList);
    QHash<DirEntry *, QString> pullFileMap() const;
    void setPullFileMap(const QHash<DirEntry *, QString> &pullFileMap);


    bool push_thread();
    bool pull_thread();
};

#endif // ADBSYNCHELPER_H
