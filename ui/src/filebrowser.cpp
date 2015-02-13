#include "filebrowser.h"
#include "ui_imagebrowser.h"
#include "QShowEvent"

ImageBrowser::ImageBrowser(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ImageBrowser)
{
    ui->setupUi(this);
    m_model = new FileSystemModel(ui->fileTree);
    ui->fileTree->setModel(m_model);    

}

ImageBrowser::~ImageBrowser()
{
    delete ui;
    delete m_model;
}

void ImageBrowser::on_finished_populating()
{
    ui->fileTree->setModel(m_model);
}

#if QT_VERSION > QT_VERSION_CHECK(5,0,0)
#include <QtConcurrent/QtConcurrentRun>
#else
#include "QtConcurrentRun"
#endif
#include "QFuture"
#include "QFutureWatcher"
#include "QMenu"


void ImageBrowser::on_fileTree_customContextMenuRequested(const QPoint &pos)
{
    QList<QAction*> list;
    if(ui->fileTree->indexAt(pos).isValid())
    {
        list.append(ui->actionObject_Info);
        DirEntry *sel = getCurrentSelection();
        if(sel != NULL && sel->isDir() == false)
        {
            list.append(ui->actionOpen);
            list.append(ui->actionDownload);
            list.append(ui->actionPerform_Download_Test);
        }
    }
    if(list.count() > 0)
        QMenu::exec(list,ui->fileTree->mapToGlobal(pos));
}

DirEntry *ImageBrowser::getCurrentSelection()
{
    QModelIndex index = ui->fileTree->currentIndex();
    DirEntry *parent = (DirEntry*)index.internalPointer();
    if(parent == NULL )
        return NULL;

    DirEntry *selection = (DirEntry*)parent->child(index.row());

    return selection;
}

void ImageBrowser::on_session_started()
{
    setEnabled(true);
    setVisible(true);

    QFuture<void> res;


//    res = QtConcurrent::run(m_client,&Client::PopulateFileList,m_model);


    QFutureWatcher<void> watcher;
    connect(&watcher,SIGNAL(finished()),SLOT(on_finished_populating()));
    watcher.setFuture(res);


}

void ImageBrowser::on_session_closed()
{
    setEnabled(false);
    setVisible(false);
    m_model->removeAll();
    m_model->clear();
    ui->fileTree->setModel(m_model);
//    ui->fileTree->update();
}


#include "imageviewergl.h"
#include "imageview.h"

void ImageBrowser::on_actionOpen_triggered()
{
    DirEntry *selection = getCurrentSelection();

    if(selection != NULL && (selection->isDir() == false))
    {
//        QImage img;
//        img = m_client->GetImage(selection);
//        if(img.isNull())
//            return;
//        ImageView imgView(img,this);
//        imgView.exec();
    }

}

void ImageBrowser::on_actionDownload_triggered()
{
    DirEntry *selection = getCurrentSelection();

//    if(selection != NULL && (selection->isDir() == false))
//        m_client->DownloadObject(selection->getObjectId());
}


void ImageBrowser::on_commandLinkButton_clicked()
{
    close();
}



void ImageBrowser::on_actionObject_Info_triggered()
{
    DirEntry *selection = getCurrentSelection();

    //** dont do if this is root*/
//    if(selection != NULL && (selection->getObjectId() != 0))
//        m_client->GetObjectInfo(selection->getObjectId());
}

void ImageBrowser::on_fileTree_doubleClicked(const QModelIndex &index)
{
    on_actionDownload_triggered();
}




void ImageBrowser::on_actionPerform_Download_Test_triggered()
{
    DirEntry *selection = getCurrentSelection();

    if(selection != NULL && (selection->isDir() == false))
    {
//        LivePreview *previewTest = new LivePreview(this);
//        previewTest->startTest(m_client,selection );
//        delete previewTest;

    }

}
