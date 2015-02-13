
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "detailedlog.h"
#include "QFileDialog"
#include "QCloseEvent"
#include "QMdiSubWindow"
#include "adbhandler.h"
#include "welcome.h"
#include "QMdiArea"

#define COMPANY_NAME "DB_SOFT"
#define PROGRAM_NAME "THENGA"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    curFile("")
{
    ui->setupUi(this);
    m_welcome  = NULL;

    ui->menuView->addAction(ui->mainToolBar->toggleViewAction());

    m_mdiArea = ui->mdiArea;

   QSettings settings(COMPANY_NAME, PROGRAM_NAME);
   int log_level = settings.value(SETTINGS_CURRENT_LOG_LEVEL,0).toInt();

   DetailedLog::Init(this)->setLogLevel(log_level);

   setupToolbar();





#ifdef _WIN32
   WORD versionWanted = MAKEWORD(1, 1);
   WSADATA wsaData;
   WSAStartup(versionWanted, &wsaData);
#endif

   m_windowMapper = new QSignalMapper(this);
   connect(m_windowMapper, SIGNAL(mapped(QWidget*)),
           this, SLOT(setActiveSubWindow(QWidget*)));

   connect(m_mdiArea,SIGNAL(subWindowActivated(QMdiSubWindow*)),SLOT(on_subwindow_changed(QMdiSubWindow*)));   




   /* show the status screen*/

   on_actionManage_triggered();
    return;
}


#include "abstractmdichild.h"

void MainWindow::updateWindowMenu()
{
    ui->menuWindows->clear();

    QList<QMdiSubWindow *> windows = m_mdiArea->subWindowList();    
    ui->menuWindows->addAction(ui->actionClose);

    ui->menuWindows->addAction(ui->actionClose_All);
    ui->menuWindows->addSeparator();

    ui->menuWindows->addAction(ui->actionCascade_Windows);
    ui->menuWindows->addAction(ui->actionTile_Windows);
    ui->menuWindows->addSeparator();


    for (int i = 0; i < windows.size(); ++i) {
        AbstractMDIChild *child = qobject_cast<AbstractMDIChild *>(windows.at(i)->widget());

        QString text;
        if (i < 9) {
            text = tr("&%1 %2").arg(i + 1)
                    .arg(child->windowTitle());
        } else {
            text = tr("%1 %2").arg(i + 1)
                    .arg(child->windowTitle());
        }
        QAction *action  = ui->menuWindows->addAction(text);
        action->setCheckable(true);
        action ->setChecked(child == activeMdiChild());
        connect(action, SIGNAL(triggered()), m_windowMapper, SLOT(map()));
        m_windowMapper->setMapping(action, windows.at(i));
    }
}


AbstractMDIChild *MainWindow::activeMdiChild()
{
    if (QMdiSubWindow *activeSubWindow = m_mdiArea->activeSubWindow())
        return qobject_cast<AbstractMDIChild *>(activeSubWindow->widget());
    return 0;
}



MainWindow::~MainWindow()
{

    delete ui;
//    delete m_mdiArea;
    delete m_windowMapper;

#ifdef _WIN32
    WSACleanup();
#endif
}




//void connectThread()

#include "QLabel"
#include "QProgressDialog"


void MainWindow::setupToolbar()
{

//    QList<QAction*> list = ui->mainSideBar->actions();

//    for(int i=0;i<list.length();i++)
//    {
//        QWidget * currentActionWidget = ui->mainSideBar->widgetForAction(list[i]);
//        ui->mainSideBar->layout()->setAlignment(currentActionWidget,Qt::AlignLeft);
//    }
//    ui->mainSideBar->setVisible(false);


////    ui->mainSideBar->;

//    ui->menuView->addAction(ui->mainSideBar->toggleViewAction());




}




bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);

    QApplication::setOverrideCursor(Qt::WaitCursor);

//    if(fileName.contains(tr(".html")) || fileName.contains(".htm"))
//        out << ui->textBrowser->toHtml();
//    else
//        out << ui->textBrowser->toPlainText();

    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
//    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}


void MainWindow::setCurrentFile(const QString &fileName)
{
    curFile = fileName;
//    ui->textBrowser->document()->setModified(false);
    setWindowModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "untitled.log";
    setWindowFilePath(shownName);
}

bool MainWindow::save()
{
    if (curFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(curFile);
    }
}

bool MainWindow::saveAs()

{
    QString defaultDir = getApplicationSettings(SETTINGS_DEFAULT_SAVE_DIR,"").toString();
    QString filterSelected;
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Log file"),
                                                    defaultDir,
                                                    tr("Web Page (*.html *.htm);; Text Files(*.txt *.log)"),
                                                    &filterSelected);
    if (fileName.isEmpty())
        return false;

    /* add the extension*/
    if(fileName.indexOf('.') == -1)
    {
        if(filterSelected.contains(tr("Text Files")))
            fileName+=tr(".log");
        else
            fileName+=tr(".html");
    }

    return saveFile(fileName);
}

void MainWindow::on_actionSave_Log_triggered()
{
    save();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
//        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::maybeSave()
{
//    if (ui->textBrowser->document()->isModified()) {
//        QMessageBox::StandardButton ret;
//        ret = QMessageBox::warning(this, tr("Thenga"),
//                     tr("The document has been modified.\n"
//                        "Do you want to save the log?"),
//                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
//        if (ret == QMessageBox::Save)
//            return save();
//        else if (ret == QMessageBox::Cancel)
//            return false;
//    }
    return true;
}

void MainWindow::on_updateLog(QString text)
{
//    ui->textBrowser->moveCursor(QTextCursor::End);
//    ui->textBrowser->insertHtml(text.replace("\n","<br>"));
//    ui->textBrowser->moveCursor(QTextCursor::End);
    qApp->processEvents();
}

void MainWindow::AppendHTMLText(QString text)
{
    emit updateLog(text);
}


QString MainWindow::getHomeLocation()
{
    QString home = QString(getenv("HOME"));
    return home;
}

void MainWindow::setActiveSubWindow(QWidget *window)
{
    if (!window)
        return;
    m_mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
    qApp->processEvents();
    m_mdiArea->update();
//    m_mdiArea-
}


QVariant MainWindow::getApplicationSettings(QString key,QVariant defaultValue)
{
     QSettings settings(COMPANY_NAME, PROGRAM_NAME);
     return settings.value(key,defaultValue);
}


void MainWindow::setApplicationSettings(QString key,QVariant value)
{
     QSettings settings(COMPANY_NAME, PROGRAM_NAME);
     settings.setValue(key,value );
}

#include "adbshell.h"
//#include "qtermwidget.h"


void MainWindow::on_actionAdb_Shell_triggered()
{
    AdbShell *as = new AdbShell;
    as->addToMDIArea(m_mdiArea);
//    QObject::connect( as->getConsole(),SIGNAL(copyAvailable(bool)),ui->actionCopy,

    connect(as->getConsole(), SIGNAL(copyAvailable(bool)),
                ui->actionCopy, SLOT(setEnabled(bool)));
//    connect(as->getConsole(), SIGNAL(copyAvailable(bool)),
//                ui->actionCut, SLOT(setEnabled(bool)));

    as->show();


}


void MainWindow::on_actionExit_triggered()
{
    qApp->exit(0);
}

void MainWindow::on_actionManage_triggered()
{
    if(m_welcome != NULL)
    {
        setActiveSubWindow(m_welcome->getSubWindow());
        return;
    }

    m_welcome = new Welcome;    
    m_welcome->addToMDIArea(m_mdiArea);
    m_welcome->showMaximized();
    QObject::connect(m_welcome,SIGNAL(destroyed()),this,SLOT(on_welcome_closed()));


}

void MainWindow::on_welcome_closed()
{
//    m_welcome->deleteLater();
    m_welcome = 0;
}

void MainWindow::on_subwindow_changed(QMdiSubWindow *window)
{
    if(window == 0)
        return;

    updateWindowMenu();

#if 0
    if(m_mdiArea->subWindowList().length() > 1)
    {
        m_mdiArea->setViewMode(QMdiArea::TabbedView);
    }
    else
    {
//        m_mdiArea->setViewMode(QMdiArea::SubWindowView);
//        m_mdiArea->activeSubWindow()->showMaximized();
    }
    m_mdiArea->activeSubWindow()->showMaximized();
#endif
}

#include "settingsdialog.h"

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog sd;
    sd.exec();
}


#include "adbfilebrowser.h"

void MainWindow::on_actionFile_Browser_triggered()
{

    AdbFileBrowser *fb = new AdbFileBrowser(this);

    /* add to mdi area, and assign delete-later*/
    fb->addToMDIArea(m_mdiArea);
    fb->populate();
    fb->show();

}

void MainWindow::on_actionAbout_triggered()
{
//    QAbout

}

void MainWindow::on_actionClose_triggered()
{
    activeMdiChild()->close();
}

void MainWindow::on_actionClose_All_triggered()
{
    m_mdiArea->closeAllSubWindows();
//    foreach (QMdiSubWindow *child, m_mdiArea->subWindowList()) {
//        child->close();
//    }

}


void MainWindow::on_actionCopy_triggered()
{
    if(activeMdiChild() == m_welcome)
        return;

    AdbShell *sh = (AdbShell*)activeMdiChild();
    sh->getConsole()->copyClipboard();
}

void MainWindow::on_actionPaste_triggered()
{
    if(activeMdiChild() == m_welcome)
        return;

    AdbShell *sh = (AdbShell*)activeMdiChild();
    sh->getConsole()->pasteClipboard();
}

#include "cpugraphview.h"

void MainWindow::on_actionCPU_Stats_triggered()
{
    AbstractMDIChild *graphDialog = new AbstractMDIChild;

    graphDialog->setWindowTitle("CPU Status");

    CPUGraphView *core0 = new CPUGraphView();
    CPUGraphView *core1 = new CPUGraphView();
    CPUGraphView *core2 = new CPUGraphView();
    CPUGraphView *core3 = new CPUGraphView();

    graphDialog->setLayout(new QVBoxLayout());
    graphDialog->layout()->addWidget(core0);
    graphDialog->layout()->addWidget(core1);
//    graphDialog->layout()->addWidget(core2);
//    graphDialog->layout()->addWidget(core3);

    core0->setSelectedComponent(0);
//    core1->setSelectedComponent(1);
//    core2->setSelectedComponent(2);
//    core3->setSelectedComponent(3);



    graphDialog->addToMDIArea(m_mdiArea);

    graphDialog->show();

//    graphDialog.show();
//    qApp->processEvents();




    core0->startTest();
//    core1->startTest();
//    core2->startTest();
//    core3->startTest();



//    graphDialog.exec();
//    delete graph;

}

void MainWindow::on_actionCascade_Windows_triggered()
{
    if(m_mdiArea != NULL)
        m_mdiArea->cascadeSubWindows();
}

void MainWindow::on_actionTile_Windows_triggered()
{
    if(m_mdiArea != NULL)
        m_mdiArea->tileSubWindows();
}

#include "QInputDialog"

void MainWindow::on_actionReboot_Device_triggered()
{
    bool ok;
    AdbHandler ah;
    QStringList options;
    options<<"Normal"<<"Recovery"<<"Bootloader";
    QString text = QInputDialog::getItem(this, "Reboot Options",
                                        "Please give reboot options",
                                        options,0,false,&ok);

    if(!ok)
        return;

    if(text.compare("Normal")==0)
        ah.rebootDevice(QString());
    else
        ah.rebootDevice(text.toLower());

}
