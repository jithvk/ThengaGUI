#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QListWidget>
#include <QtNetwork>
#include "QMessageBox"
#include "QProgressBar"
#include "filebrowser.h"
#include "QMdiSubWindow"
#include "welcome.h"




class AbstractMDIChild;


namespace Ui {
    class MainWindow;
}

/* strings for saving settings*/
#define SETTINGS_CURRENT_LOG_LEVEL "current_log_level"
#define SETTINGS_GUID_ARRAY        "guids"
#define SETTINGS_GUID_VALUE        "guid"
#define SETTINGS_DEFAULT_SERVER_ADDRESS "default_server_address"
#define SETTINGS_TEST_LEVEL        "test_level"
#define SETTINGS_FRIENDLY_NAME     "friendly_name"
#define SETTINGS_DEFAULT_SAVE_DIR  "default_save_dir"



class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    bool save();
    bool saveAs();

    void closeEvent(QCloseEvent *event);

    bool maybeSave();

    void AppendHTMLText(QString text);

//    /* static functions*/
    static QVariant getApplicationSettings(QString key,QVariant defaultValue = 0);
    static void setApplicationSettings(QString key,QVariant value);

    void enableToolbars(bool enable);







public slots:
    AbstractMDIChild *activeMdiChild();
signals:
    void updateLog(QString txt);

private slots:
    void on_updateLog(QString text);

    void updateWindowMenu();
    void on_actionSave_Log_triggered();
    void setActiveSubWindow(QWidget *window);


    void on_actionAdb_Shell_triggered();    

    void on_actionExit_triggered();
    void on_actionManage_triggered();

    void on_welcome_closed();
    void on_subwindow_changed(QMdiSubWindow*window);

    void on_actionSettings_triggered();

    void on_actionFile_Browser_triggered();

    void on_actionAbout_triggered();

    void on_actionClose_triggered();

    void on_actionClose_All_triggered();


    void on_actionCopy_triggered();

    void on_actionPaste_triggered();

    void on_actionCPU_Stats_triggered();

    void on_actionCascade_Windows_triggered();

    void on_actionTile_Windows_triggered();

    void on_actionReboot_Device_triggered();

private:

    Ui::MainWindow *ui;
    void setupToolbar();
    QString curFile;
    QSignalMapper *m_windowMapper;
    Welcome *m_welcome;
    QMdiArea *m_mdiArea;



    QString getHomeLocation();
};

#endif // MAINWINDOW_H
