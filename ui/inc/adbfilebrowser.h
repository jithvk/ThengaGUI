#ifndef ADBFILEBROWSER_H
#define ADBFILEBROWSER_H

#include "abstractmdichild.h"
#include "fileobject.h"
#include "filesystemmodel.h"
#include "QTreeView"
#include "QDockWidget"
#include "filesystemfilter.h"
#include "QStack"
#include "filelistview.h"

namespace Ui {
class AdbFileBrowser;
}

class AdbFileBrowser : public AbstractMDIChild
{
    Q_OBJECT
    
public:
    explicit AdbFileBrowser(QWidget *parent = 0);
    ~AdbFileBrowser();
    void populate();
    void setupUI();
    void changeDir(uint dirId);
    void navigateBack();

    QListView *getListView()
    {
        return m_fileListView;
    }

signals:
    void droppedUrls(QStringList);


private slots:
    void on_fileView_onclicked(const QModelIndex &index);
    void refresh_item(const QModelIndex &index);

    void on_listView_doubleClicked(const QModelIndex &index);

    void on_actionBack_triggered();

    void on_fileView_customContextMenuRequested(const QPoint &pos);

    void on_actionView_List_triggered(bool checked);

    void on_actionDownload_triggered();

    void on_droppedUrls(QStringList list);

    void on_actionUp_triggered();

    void on_actionDelete_triggered();

    void on_actionNew_Directory_triggered();

    void on_actionRename_triggered();

private:
    Ui::AdbFileBrowser *ui;
    FileSystemModel *m_model;
    QTreeView   *m_fileTreeView;
    QListView   *m_fileListView;

    FileSystemFilter *m_treeFilter;


    QStack<uint> m_dirStack;

    // QWidget interface
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *e);




    // QWidget interface
protected:
    void dragMoveEvent(QDragMoveEvent *);
};

#endif // ADBFILEBROWSER_H
