#include "filelistview.h"
#include "QDrag"
#include "direntry.h"
#include "QMouseEvent"
#include "QMimeData"
#include "QDebug"
#include "QApplication"
#include "QTemporaryFile"

DragDropMimeData::DragDropMimeData()
    :QMimeData()
{
    m_supportedFormats.empty();
    m_supportedFormats.append("text/uri-list");
    m_supportedFormats.append("application/octet-stream");

}

QStringList DragDropMimeData::formats() const
{
    return m_supportedFormats;
}

QVariant DragDropMimeData::retrieveData(const QString &format, QVariant::Type preferredType) const
{
    qDebug()<<"Retrive data " <<format<<"Type "<<preferredType;


    if(format.compare("text/uri-list")==0)
    {
        QTemporaryFile file(m_fileName);
        file.open();
        return QUrl::fromLocalFile(file.fileName());
    }

    return QMimeData::retrieveData(format,preferredType);
}


FileListView::FileListView(QWidget *parent) :
    QListView(parent)
{
}

void FileListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
            m_startPos = event->pos();

    QListView::mousePressEvent(event);
}

void FileListView::mouseReleaseEvent(QMouseEvent *)
{
}


void FileListView::performDragOut()
{
    QModelIndex index =  currentIndex();

    DirEntry *parent = (DirEntry*)index.internalPointer();
    if(parent == NULL )
        return ;

    DirEntry *item = (DirEntry*)parent->child(index.row());
    if(item == NULL)
        return;

       if (item) {
           DragDropMimeData *mimeData = new DragDropMimeData;
           mimeData->m_fileName = item->getName();


           QDrag *drag = new QDrag(this);
           drag->setMimeData(mimeData);
           drag->setPixmap(item->icon().pixmap(64,64));
           qDebug()<<"Trying to drag "<<item->getName();
           if (drag->exec(Qt::CopyAction) == Qt::CopyAction)
//               delete item;
                qDebug()<<"Copied "<<item->getName();
       }

}

void FileListView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
            int distance = (event->pos() - m_startPos).manhattanLength();
            if (distance >= QApplication::startDragDistance())
                performDragOut();
        }
    QListView::mouseMoveEvent(event);
}






bool DragDropMimeData::hasFormat(const QString &mimetype) const
{
    if(m_supportedFormats.contains(mimetype))
        return true;

    return false;
}
