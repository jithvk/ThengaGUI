#include "ui/inc/adbfilebrowser.h"
#include "ui_adbfilebrowser.h"


#include "QToolBar"
#include "QSpacerItem"
#include "QSplitter"

#include "fileobject.h"
#include "QDockWidget"
#include "direntry.h"
#include "QDebug"
#include "filesystemfilter.h"
#include "QMessageBox"

#define delete_if_valid(x) {if(x!=NULL)delete(x);x=NULL;}

AdbFileBrowser::AdbFileBrowser(QWidget *parent) :
    AbstractMDIChild(parent),
    ui(new Ui::AdbFileBrowser)
{

    m_model = NULL;
    m_fileTreeView = NULL;
    m_treeFilter = NULL;
    m_fileListView = NULL;    

    ui->setupUi(this);
    setupUI();

    connect(this,SIGNAL(droppedUrls(QStringList)),
            this,SLOT(on_droppedUrls(QStringList)),Qt::QueuedConnection);

}


void AdbFileBrowser::setupUI()
{

    QSplitter *splitter = new QSplitter(this);


    m_fileTreeView = new QTreeView;
    m_fileTreeView->setIconSize(QSize(32,32));
    m_fileTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    m_fileTreeView->setMaximumWidth(300);


    m_fileListView = new QListView;
    m_fileListView->setViewMode(QListView::IconMode);
    m_fileListView->setIconSize(QSize(32,32));
    m_fileListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_fileListView->setResizeMode(QListView::Adjust);
    m_fileListView->setGridSize(QSize(64,64));
    m_fileListView->setEditTriggers(QListView::EditKeyPressed);
    m_fileListView->setContextMenuPolicy(Qt::CustomContextMenu);

    setAcceptDrops(true);


    splitter->insertWidget(0,(QWidget*)m_fileTreeView);

    QToolBar *toolbar = new QToolBar("Navigation bar",this);
    toolbar->addAction(ui->actionBack);
    toolbar->addAction(ui->actionUp);
    toolbar->addAction(ui->actionView_List);
    toolbar->addAction(ui->actionNew_Directory);
    toolbar->addAction(ui->actionDelete);
//    toolbar->addAction(ui->action);

    QVBoxLayout *insideLayout = new QVBoxLayout(splitter);
    QWidget *insideWidget = new QWidget(splitter);

    insideWidget->setLayout(insideLayout);

    insideLayout->insertWidget(0,toolbar);
    insideLayout->insertWidget(1,m_fileListView);

    splitter->insertWidget(1,(QWidget*)insideWidget);
    splitter->setStretchFactor(0,10);
    splitter->setStretchFactor(1,30);

    ui->verticalLayout->addWidget(splitter);    


    QObject::connect(m_fileTreeView,SIGNAL(clicked(QModelIndex)),
                     this,SLOT(on_fileView_onclicked(QModelIndex)));

    QObject::connect(m_fileTreeView,SIGNAL(doubleClicked(QModelIndex)),
                     this,SLOT(refresh_item(QModelIndex)));

    QObject::connect(m_fileListView,SIGNAL(customContextMenuRequested(QPoint)),
                     this,SLOT(on_fileView_customContextMenuRequested(QPoint)));


    QObject::connect(m_fileListView,SIGNAL(doubleClicked(QModelIndex)),
                     this,SLOT(on_listView_doubleClicked(QModelIndex)));



}



AdbFileBrowser::~AdbFileBrowser()
{



    delete ui;
    m_model->deleteLater();
    m_model = NULL;

//    delete_if_valid(m_treeFilter);
//    delete_if_valid(m_fileTreeView) ;
//    delete_if_valid(m_fileListView)




}

void AdbFileBrowser::populate()
{
    if(m_model != NULL)
        delete m_model;


    m_model = new FileSystemModel(this);

    m_model->AddRootDir(0,"/");



    m_treeFilter = new FileSystemFilter;
    m_treeFilter->setFilterType(DIR_ONLY);
    m_treeFilter->setSourceModel(m_model);

    m_fileTreeView->setModel(m_treeFilter);

    changeDir(0);

}



void AdbFileBrowser::on_fileView_onclicked(const QModelIndex &index)
{

    QModelIndex ind = m_treeFilter->mapToSource(index);


    DirEntry *parent = (DirEntry*)ind.internalPointer();
    if(parent == NULL )
        return ;

    DirEntry *selection = (DirEntry*)parent->child(ind.row());
    if(selection == NULL)
        return;

    changeDir(selection->getObjectId());

}


/* refresh the current selection item*/
void AdbFileBrowser::refresh_item(const QModelIndex &index)
{

    QModelIndex ind = m_treeFilter->mapToSource(index);


    DirEntry *parent = (DirEntry*)ind.internalPointer();
    if(parent == NULL )
        return ;

    DirEntry *selection = (DirEntry*)parent->child(ind.row());
    if(selection == NULL)
        return;

    DirEntry *dir = m_model->FindItem(selection->getObjectId());

    if(dir == NULL)
        return;

    dir->setWalked(false);
    dir->populateChildren(m_model);

    changeDir(selection->getObjectId());

}


void AdbFileBrowser::changeDir(uint dirId)
{

    uint currentDir = m_model->getCurrentDir();

    QStandardItemModel *model = m_model->getDirItemsModel(dirId);
    if(model == NULL)
        return;

    getListView()->setModel(model);
    if(currentDir != dirId)
        m_dirStack.push(currentDir);

    QModelIndex listIndex = m_treeFilter->mapFromSource(m_model->FindItem(dirId)->index());

    m_fileTreeView->setCurrentIndex(listIndex);
    setWindowTitle("File Browser ["+m_model->FindItem(dirId)->getAbsPath()+"]");

}

void AdbFileBrowser::navigateBack()
{
    if(m_dirStack.isEmpty())
        return;

    /* get the previous one*/
    uint lastDir = m_dirStack.pop();

    getListView()->setModel(NULL);

    QStandardItemModel *model = m_model->getDirItemsModel(lastDir);
    getListView()->clearSelection();
    getListView()->setModel(model);

    qDebug()<<"Going back to dir "<<lastDir;
}




void AdbFileBrowser::on_listView_doubleClicked(const QModelIndex &index)
{
    if(index.isValid() == false)
        return;

    FileSystemModel *model = (FileSystemModel*) m_fileListView->model();

    DirEntry *selection = (DirEntry*)model->itemFromIndex(index);

    if(selection == NULL)
        return;

    changeDir(selection->getObjectId());

}

void AdbFileBrowser::on_actionBack_triggered()
{
    navigateBack();
}

#include "QMenu"

void AdbFileBrowser::on_fileView_customContextMenuRequested(const QPoint &pos)
{
    QList<QAction*> list;
    list.append(ui->actionDownload);

    if(m_fileListView->indexAt(pos).isValid())
    {
        list.append(ui->actionCopy);
        list.append(ui->actionCut);
        list.append(ui->actionDelete);
        list.append(ui->actionRename);


    }

    list.append(ui->actionPaste);
    list.append(ui->actionNew_Directory);


    if(list.count() > 0)
        QMenu::exec(list,m_fileListView->mapToGlobal(pos));
}

#include "QDragEnterEvent"
#include "QMimeData"
#include "QDropEvent"
#include "QDragLeaveEvent"
#include "QProgressDialog"

void AdbFileBrowser::dragEnterEvent(QDragEnterEvent *event)
{

    qDebug()<<"Drag Enter Event";
    if (event->mimeData()->hasUrls()) {
            event->acceptProposedAction();
        }
}



void AdbFileBrowser::dropEvent(QDropEvent *e)
{

    qDebug() << "Dropped Something :" << e->type();
    if(e->mimeData()->urls().isEmpty())
        return;

    QStringList fileList;
    foreach (const QUrl &url, e->mimeData()->urls()) {
           const QString &fileName = url.toLocalFile();
           if(fileName.isEmpty())
               continue;
           fileList.append(fileName);
    }

    emit droppedUrls(fileList);
    if(fileList.isEmpty() == false)
        e->accept();

}

void AdbFileBrowser::on_actionView_List_triggered(bool checked)
{
    if(checked)
    {
        getListView()->setViewMode(QListView::ListMode);
        m_fileListView->setGridSize(QSize(32,32));
    }
    else
    {
        m_fileListView->setGridSize(QSize(64,64));
        getListView()->setViewMode(QListView::IconMode);
    }
}



#include "QFileDialog"

void AdbFileBrowser::on_actionDownload_triggered()
{

    QStandardItemModel *model = (QStandardItemModel*)m_fileListView->model();

    QList<FileObjectID> list;
    foreach(const QModelIndex &index,
            m_fileListView->selectionModel()->selectedIndexes())
    {
        DirEntry *entry = (DirEntry*)model->itemFromIndex(index);
        list.append(entry->getObjectId());
    }


    if(list.isEmpty())
        return;

    QString dirName = QFileDialog::getExistingDirectory();

    if(dirName.isEmpty())
        return;

    DirEntry::pullFiles(m_model,list,dirName);

}

void AdbFileBrowser::on_droppedUrls(QStringList list)
{
    DirEntry *entry = m_model->FindItem(m_model->getCurrentDir());

    entry->pushFiles(list);
    changeDir(entry->getObjectId());

}

void AdbFileBrowser::on_actionUp_triggered()
{
    DirEntry *entry = m_model->FindItem(m_model->getCurrentDir());

    changeDir(entry->getParentId());
}


void AdbFileBrowser::dragMoveEvent(QDragMoveEvent *)
{
    qDebug()<<"Drag Move Event";
}

void AdbFileBrowser::on_actionDelete_triggered()
{
    DirEntry *currentDir = m_model->FindItem(m_model->getCurrentDir());

    QStandardItemModel *model = (QStandardItemModel*)m_fileListView->model();

    QList<FileObjectID> list;
    foreach(const QModelIndex &index,
            m_fileListView->selectionModel()->selectedIndexes())
    {
        DirEntry *entry = (DirEntry*)model->itemFromIndex(index);
        list.append(entry->getObjectId());
    }

    if(list.isEmpty())
        return;

    if(currentDir->deleteFiles(list) == false)
        QMessageBox::critical(this,"Error","Failed to delete file(s)");

    changeDir(currentDir->getObjectId());
}

#include "adbhandler.h"
#include "QInputDialog"

void AdbFileBrowser::on_actionNew_Directory_triggered()
{
     DirEntry *currentDir = m_model->FindItem(m_model->getCurrentDir());

     QString name = QInputDialog::getText(this,"Directory Name","Give a name for new Directory");
     if(name.isEmpty())
         return;
     AdbHandler ah;
     name = ah.executeCommand("mkdir",currentDir->getAbsPath()+"/"+name);
     if(name.isEmpty() == false)
     {
         QMessageBox::critical(this,"Error","Failed to make directory :"+name);
         return;
     }
     currentDir->setWalked(false);
     changeDir(currentDir->getObjectId());

}

void AdbFileBrowser::on_actionRename_triggered()
{
    DirEntry *currentDir = m_model->FindItem(m_model->getCurrentDir());

    QStandardItemModel *model = (QStandardItemModel*)m_fileListView->model();

    QList<FileObjectID> list;

    foreach(const QModelIndex &index,
            m_fileListView->selectionModel()->selectedIndexes())
    {
        DirEntry *entry = (DirEntry*)model->itemFromIndex(index);
        list.append(entry->getObjectId());
    }

    if(list.count() != 1)
    {
        m_fileListView->clearSelection();
        return;
    }

    DirEntry *entry = m_model->FindItem(list.at(0));


    QString name = QInputDialog::getText(this,"Directory Name","Give a name for new Directory",
                                         QLineEdit::Normal,entry->getName());
    if(name.isEmpty())
        return;

    AdbHandler ah;
    name = ah.executeCommand("mv ",entry->getAbsPath(),currentDir->getAbsPath()+"/"+name);
    if(name.isEmpty() == false)
    {
        QMessageBox::critical(this,"Error","Failed to rename :"+name);
        return;
    }

    currentDir->setWalked(false);
    changeDir(currentDir->getObjectId());

}
