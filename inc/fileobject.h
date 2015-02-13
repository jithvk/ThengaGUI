#ifndef FILEOBJECT_H
#define FILEOBJECT_H


#include <QObject>
#include "stdint.h"
#include <sys/stat.h>

typedef qlonglong FileObjectID;
#define FILE_OBJECT_INVALID_ID   ((FileObjectID)-1)

#define	S_ISLINK(m)	(((m) & S_IFMT) == 0xA000)

class FileObject : public QObject
{
    Q_OBJECT


public:
    explicit FileObject(QObject *parent = 0);
    ~FileObject();
    FileObject(const FileObject &dummy);
    FileObject & operator=(const FileObject &dummy);
signals:

private:

public:
//    ObjectInfo objectInfo;
    FileObjectID objectID;

    /* Object Info*/
    QString m_fileName;
    ulong m_fileSize;
    ulong m_parentObject;
    uint m_associationType;
    ulong m_time;

/* functions*/
public:
    bool isDir()
    {
        return S_ISDIR(m_associationType);
    }

    bool isLink()
    {
        return S_ISLINK(m_associationType);
    }

    bool isRegular()
    {
        return S_ISREG(m_associationType);
    }

public slots:
    
};

#endif // FILEOBJECT_H
