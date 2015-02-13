#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QStandardItemModel>
#include "direntry.h"
#include "QStringListModel"

enum MimeType
{
    FILE_TYPE_IMAGE,
    FILE_TYPE_VIDEO,
    FILE_TYPE_OTHER
};

#define FILE_SYSTEM_MODEL_MIME_TYPE "application/vnd.tenga.direntry"
//class FileSystemModel;

class FileSystemModel : public QStandardItemModel
{
    Q_OBJECT

public:
    FileSystemModel(QObject *parent = 0);
    void AddRootDir(FileObjectID rootObjectID, QString rootDir = QString("CAMERA_NAME"));    
    bool Add(FileObject &obj);
    ~FileSystemModel();
    void removeAll();
    DirEntry *FindItem(FileObjectID objectID);

    QList<DirEntry*> getAllObjects();
//    qlonglong getCount();


    FileObjectID getCurrentDir();


private:
    DirEntry *m_rootDir;
    QHash<FileObjectID, DirEntry *> m_dirHashTable;
    bool AddObject(FileObject obj);

    QIcon m_folderIcon;
    QIcon m_imageIcon;
    QIcon m_otherIcon;
    qlonglong m_objIdCounter;
    FileSystemModel *m_currentDirListModel;
    qlonglong m_curDirId;




public:
    FileSystemModel *getDirItemsModel(FileObjectID dirObjectId);



public:
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;
    Qt::DropActions supportedDragActions() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;


public:
    QMimeData *mimeData(const QModelIndexList &indexes) const;


public:
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    bool removeObject(FileObjectID id);

public slots:
//    void onItemInserted(QModelIndex parent, int start, int end);
//    void onItemsRemoved(QModelIndex parent, int start, int end);


public:


    qlonglong getCount();
};

#endif // FILESYSTEMMODEL_H
