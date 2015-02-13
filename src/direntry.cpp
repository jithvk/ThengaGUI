#include "direntry.h"
#include "QObject"
#include "QDebug"

#include "adbhandler.h"
#include "filesystemmodel.h"


DirEntry::DirEntry() :
    QStandardItem()
{
    m_walked = false;
    m_parentId = 0;
    m_size = m_mode = m_time = 0;
}

DirEntry::DirEntry(QString name, FileObjectID objID, DirEntry *parent, EntryType type)
:QStandardItem(name)
{
    m_name = name;

    m_objectID = objID;

    m_parentId = 0;
    m_walked = false;
    m_size = m_mode = m_time = 0;

    if(type != DIR)
        m_mode|=S_IFREG;

    setParent(parent);

}

DirEntry::DirEntry(FileObject obj, DirEntry *parent)
    :QStandardItem(obj.m_fileName)
{
    m_name = obj.m_fileName;
//    m_type = obj.isRegular()?FILE:DIR; /* default is dir, change this for files*/
    m_objectID = obj.objectID;
    m_size = obj.m_fileSize;
    m_mode = obj.m_associationType;
    m_time = obj.m_time;


    m_parentId = 0;
    m_walked = false;

    setParent(parent);
}

void DirEntry::setParent(DirEntry *parent)
{
    QString path;

    if(parent != NULL)
    {
        m_parentId = parent->getObjectId();
        path = parent->getAbsPath();
    }

    /* Check for root dir*/
    if(path.isEmpty())
        m_absPath = m_name;
    else
    {
        if(path.endsWith("/"))
            m_absPath = QString("%1%2").arg(path).arg(m_name);
        else
            m_absPath = QString("%1/%2").arg(path).arg(m_name);
    }

}

DirEntry::~DirEntry()
{    
//    RemoveAll();
//    qDebug()<<"Deleted "+getAbsPath();
}

QString DirEntry::getAbsPath()
{
    return m_absPath;
}


void DirEntry::populateChildren(FileSystemModel *model)
{


    if(iswalked() == false && isDir())
    {
        AdbHandler ah;
        foreach (FileObject obj, ah.getDirListing(getAbsPath())) {
            obj.m_parentObject = getObjectId();
            model->Add(obj);
        }
        setWalked(true);
    }
}


bool DirEntry::iswalked() const
{
    return m_walked;
}

void DirEntry::setWalked(bool walked)
{
    m_walked = walked;
    if(walked == false && hasChildren())
    {
        FileSystemModel *_mod = (FileSystemModel*)QStandardItem::model();
        int i= rowCount();
        while(i>0)
        {
            i--;
            DirEntry*entry = (DirEntry*)child(i);
            _mod->removeObject(entry->getObjectId());
        }
    }
}


QList<DirEntry*> DirEntry::getChildren(bool populate)
{
    QList<DirEntry*> list;

    FileSystemModel *model = (FileSystemModel*)QStandardItem::model();

    if(model!=NULL && populate)
        populateChildren(model);

    if(hasChildren() == false)
        return list;

    for(int i=0;i<rowCount();i++)
    {
        DirEntry *ch = (DirEntry*)child(i);
        list.append(ch);
        list.append(ch->getChildren(populate));
    }

    return list;

}



#include "QDebug"

void DirEntry::pullFiles(FileSystemModel *model, QList<FileObjectID> fileList, QString targetDir)
{
    if(model == NULL || fileList.isEmpty() | targetDir.isEmpty() )
        return;


    QHash<DirEntry*,QString> fileMap;
    foreach (FileObjectID id, fileList ) {
        DirEntry *item = model->FindItem(id);

        QList<DirEntry*> fileList;
        fileList.append(item);

        if(item->isDir())
        {
            item->populateChildren(model);
            fileList.append(item->getChildren());
        }
        foreach (DirEntry*entry, fileList) {
            QString targetFile = entry->getAbsPath().replace(item->getAbsPath(),
                                                             targetDir+"/"+item->getName());
            if(entry->isDir())
                QDir(targetFile).mkpath(".");
            else
                fileMap.insert(entry,targetFile);
        }
        fileList.clear();
    }

    AdbHandler().pull(fileMap);

}

bool DirEntry::pushFile(QString localFile)
{
    return pushFiles(QStringList(localFile));
}


bool DirEntry::pushFiles(QStringList url)
{
    if(isDir() == false)
        return false;

    AdbHandler ah;
    bool ret =  ah.push(url,getAbsPath());
//    RemoveAll();
    setWalked(false);

    return ret;

}


bool DirEntry::deleteFiles( QList<FileObjectID> fileList)
{
    FileSystemModel *fsModel = (FileSystemModel*)QStandardItem::model();

    if(fsModel == NULL)
        return false;

    bool ret = true;
    AdbHandler handler;

    foreach (FileObjectID id, fileList ) {
        DirEntry *item = fsModel->FindItem(id);
        if(item!= NULL)
            if(handler.deleteFile(item->getAbsPath()) == false)
                ret = false;
            else
                fsModel->removeObject(item->getObjectId());
    }

    setWalked(false);
    populateChildren(fsModel);
    return ret;

}


