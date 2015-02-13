#include "adbshell.h"
//#include "ui_adbshell.h"
#include "qtermwidget.h"
#include "mainwindow.h"

#include "QFont"

AdbShell::AdbShell(QWidget *parent) :
    AbstractMDIChild(parent)
{
    m_console = new QTermWidget();

    QFont font;// = QApplication::font();
#ifdef Q_WS_MAC
    font.setFamily("Monaco");
#elif defined(Q_WS_QWS)
    font.setFamily("fixed");
#elif defined(_WIN32)
    font.setFamily("Courier");
#else
    font.setFamily("Monospace");
#endif
    font.setPointSize(12);
    font.setStyle(QFont::StyleNormal);
    font.setStyleHint(QFont::TypeWriter);
    font.setWeight(QFont::DemiBold);
    m_console->setTerminalFont(font);

    QString colorScheme = MainWindow::getApplicationSettings("ColorScheme",QString("Linux")).toString();

    m_console->setColorScheme(colorScheme);
    m_console->setScrollBarPosition(QTermWidget::ScrollBarRight);

    // info output
        qDebug() << "* INFO *************************";
        qDebug() << " availableKeyBindings:" << m_console->availableKeyBindings();
        qDebug() << " keyBindings:" << m_console->keyBindings();
        qDebug() << " availableColorSchemes:" << m_console->availableColorSchemes();
        qDebug() << "* INFO END *********************";

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_console);
    this->setLayout(layout);
    setWindowTitle("Shell");


    QObject::connect(m_console, SIGNAL(finished()), this, SLOT(removeWindow()));


}

AdbShell::~AdbShell()
{
//    delete ui;
    delete m_console;
}

QTermWidget *AdbShell::getConsole()
{
    return m_console;
}
