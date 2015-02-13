#ifndef FILEBROWSER_H
#define FILEBROWSER_H

#include <QDialog>
#include <QDockWidget>
#include "filesystemmodel.h"



namespace Ui {
class ImageBrowser;
}

class ImageBrowser : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit ImageBrowser(QWidget *parent = 0);
    ~ImageBrowser();

    inline FileSystemModel *getModel()
    {
        return m_model;
    }


    void on_session_started();
    void on_session_closed();
    
private slots:
    void on_finished_populating();
    void on_fileTree_customContextMenuRequested(const QPoint &pos);
    void on_actionOpen_triggered();
    void on_actionDownload_triggered();
    void on_commandLinkButton_clicked();
    void on_actionObject_Info_triggered();


    void on_fileTree_doubleClicked(const QModelIndex &index);

    void on_actionPerform_Download_Test_triggered();

private:
    Ui::ImageBrowser *ui;
    FileSystemModel *m_model;
    DirEntry *getCurrentSelection();

};

#endif // FILEBROWSER_H
