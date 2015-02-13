#include "filesystemmodel.h"
#include "QStringListModel"
#include "QDebug"
#include "QMimeData"

FileSystemModel::FileSystemModel(QObject *parent):
QStandardItemModel(parent),
  m_folderIcon(":icons/res/folder.png"),
  m_imageIcon(":icons/res/file.png"),
  m_otherIcon(":icons/res/link.png"),
  m_objIdCounter(0)
{
    m_rootDir = NULL;
    m_currentDirListModel = NULL;
    m_curDirId =0;

//    QObject::connect(this,SIGNAL(rowsInserted(QModelIndex,int,int)),
//            this,SLOT(onItemInserted(QModelIndex,int,int)));

}


#define delete_if_valid(x) {if(x!=NULL)delete(x);x=NULL;}

FileSystemModel::~FileSystemModel()
{
    removeAll();    
//    delete_if_valid(m_rootDir);
    delete_if_valid(m_currentDirListModel);

}

void FileSystemModel::AddRootDir(FileObjectID rootObjectID, QString rootDir)
{
    m_rootDir = new DirEntry(rootDir,rootObjectID,NULL);


    m_rootDir->setIcon(m_folderIcon);

    m_objIdCounter = rootObjectID +1;

    setHorizontalHeaderLabels(QStringList(rootDir));

    QStandardItemModel::appendRow(m_rootDir);

    m_dirHashTable.insert(rootObjectID,m_rootDir);
}

/*  Inut: dirname - just the name of the dir, not the abs path eg:-ImageFolder
  * parentDir - Abs path of parent dir eg:- /folder/current/path
  *
  */
bool FileSystemModel::AddObject(FileObject obj)
{
    DirEntry *parent = NULL,*current;

    if(m_dirHashTable.contains(obj.objectID))
     {
         qDebug("Already added dir");
         return true; /* dir already there*/
     }

    if(!m_dirHashTable.contains(obj.m_parentObject))
    {
        qDebug()<<"Invalide parent path";
        return false; /* Invalid parent*/
    }

    parent = m_dirHashTable.value(obj.m_parentObject);


    current = new DirEntry(obj,parent);

    if(obj.isRegular())
        current->setIcon(m_imageIcon);
    else if(obj.isDir())
        current->setIcon(m_folderIcon);
    else if(obj.isLink())
        current->setIcon(m_otherIcon);


    if(parent != NULL)
        parent->appendRow(current);

    m_dirHashTable.insert(current->getObjectId(),current);



    return true;
}


/* should remove this obj and all its children*/
bool FileSystemModel::removeObject(FileObjectID id)
{
    if(m_dirHashTable.contains(id)==false)
        return true;

    DirEntry *selection  = FindItem(id);

    if(selection != NULL)
    {
        if(selection->hasChildren())
        {
            foreach (DirEntry* child, selection->getChildren(false)) {
                m_dirHashTable.remove(child->getObjectId());
            }
        }
        m_dirHashTable.remove(selection->getObjectId());
    }

    QModelIndex index = indexFromItem(selection);

    if(index.isValid())
        removeRow(index.row(),index.parent());

}


bool FileSystemModel::Add(FileObject &obj)
{
    if(obj.objectID == 0)
        return false;

    if(obj.objectID == FILE_OBJECT_INVALID_ID)
    {
        obj.objectID = m_objIdCounter++;
    }

    return  AddObject(obj);
}

qlonglong FileSystemModel::getCount()
{
    return m_dirHashTable.size();
}

FileObjectID FileSystemModel::getCurrentDir()
{
    return m_curDirId;
}


QList<DirEntry*> FileSystemModel::getAllObjects()
{
    return m_dirHashTable.values();
}

DirEntry *FileSystemModel::FindItem(FileObjectID objectID)
{
    if(m_dirHashTable.contains(objectID))
        return  m_dirHashTable.value(objectID);

    return NULL;
}


/* Also makes dirObjectId the current dir*/
FileSystemModel *FileSystemModel::getDirItemsModel(FileObjectID dirObjectId)
{
    if(m_dirHashTable.contains(dirObjectId) == false)
       return NULL;


    delete_if_valid(m_currentDirListModel);
//    m_currentDirListModel.removeAll();


    DirEntry *current = FindItem(dirObjectId);

    if( current->iswalked() == false)
        current->populateChildren(this);

    int i = current->rowCount();

    /* create item with this as parent*/
    m_currentDirListModel = new FileSystemModel(this);

    while(--i>=0)                
    {        
        DirEntry *item = (DirEntry*)current->child(i);
        DirEntry *newItem = new DirEntry(item->getName(),item->getObjectId(),
                         current,item->isDir()?DirEntry::DIR:DirEntry::FILE);
        newItem->setIcon(item->icon());

        m_currentDirListModel->QStandardItemModel::appendRow(newItem);
    }

    m_curDirId = current->getObjectId();


    return m_currentDirListModel;
}


void FileSystemModel::removeAll()
{
    m_dirHashTable.clear();
    clear();

    m_rootDir = NULL;
}




QStringList FileSystemModel::mimeTypes() const
{
    return QStringList(FILE_SYSTEM_MODEL_MIME_TYPE);
}

Qt::DropActions FileSystemModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions FileSystemModel::supportedDragActions() const
{
    return Qt::CopyAction | Qt::MoveAction;;
}

Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);

       if (index.isValid())
           return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
       else
           return Qt::ItemIsDropEnabled | defaultFlags;
}


QMimeData *FileSystemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
   QByteArray encodedData;

   QDataStream stream(&encodedData, QIODevice::WriteOnly);

   foreach (const QModelIndex &index, indexes) {
       if (index.isValid()) {
           DirEntry *entry = (DirEntry*)QStandardItemModel::itemFromIndex(index);
           if(entry == NULL)
               continue;
           stream << entry->getAbsPath();
       }
   }

   mimeData->setData(FILE_SYSTEM_MODEL_MIME_TYPE, encodedData);
   return mimeData;
}


bool FileSystemModel::dropMimeData(const QMimeData *data,
                                   Qt::DropAction action,
                                   int row, int column,
                                   const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat(FILE_SYSTEM_MODEL_MIME_TYPE))
        return false;

    int beginRow = -1;

    if (row != -1)
        beginRow = row;

    else if (parent.isValid())
        beginRow = parent.row();

    else
        beginRow = rowCount(QModelIndex());

    QByteArray encodedData = data->data(FILE_SYSTEM_MODEL_MIME_TYPE);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList newItems;
    int rows = 0;

    while (!stream.atEnd()) {
        QString text;
        stream >> text;
        newItems << text;
        ++rows;
    }

    qDebug()<<newItems.join(", ");

    return true;

}


//void FileSystemModel::onItemInserted( QModelIndex parent, int start, int end)
//{
////    qDebug()<<"Items inserted "<<start<<end;

//    DirEntry *parentEntry = (DirEntry*)itemFromIndex(parent);
//    if(parentEntry == NULL)
//        parentEntry = (DirEntry*)invisibleRootItem();

//    while(start<=end)
//    {
//        DirEntry *selection = (DirEntry*)parentEntry->child(start);
//        if(selection != NULL)
//        {
//            m_dirHashTable.insert(selection->getObjectId(),selection);
//        }
//        start++;
//    }

//}

