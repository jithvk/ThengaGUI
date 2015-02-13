#ifndef DIRENTRY_H
#define DIRENTRY_H


#include "fileobject.h"
#include <QStandardItem>


class FileSystemModel;


class DirEntry : public QStandardItem
{
public:
    enum EntryType
    {
        FILE,
        DIR
    };

public:
    DirEntry();

    DirEntry(QString name,FileObjectID objID,DirEntry *parent,EntryType type = DIR);
    DirEntry(FileObject obj,DirEntry *parent);


//    void RemoveAll();
    ~DirEntry();
    QString getAbsPath();    

    bool isDir() { return !S_ISREG(m_mode); }
    QString getName() { return m_name;  }
    FileObjectID getObjectId() {  return m_objectID; }
    FileObjectID getParentId() { return m_parentId; }
    ulong getFileSize(){return m_size;}
    ulong getTime(){return m_time; }
    ulong getMode(){return m_mode; }





    bool iswalked() const;
    void setWalked(bool iswalked);    

    void populateChildren(FileSystemModel *model);
    bool pushFile(QString localFile);
    static void pullFiles(FileSystemModel *model,QList<FileObjectID> fileList, QString targetDir);
    QList<DirEntry *> getChildren(bool populate = true);
    bool pushFiles(QStringList url);
    bool deleteFiles(QList<FileObjectID> fileList);
private:
    void setParent(DirEntry *parent);


private:
    FileObjectID m_objectID;
    QString m_absPath; /* full name and path*/
    QString m_name;
    FileObjectID m_parentId;

    bool m_walked;
    ulong m_mode;
    ulong m_time;
    ulong m_size;

signals:

public slots:

};

#endif // DIRENTRY_H
