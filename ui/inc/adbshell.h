#ifndef ADBSHELL_H
#define ADBSHELL_H

#include <QDialog>
#include "abstractmdichild.h"
#include "qtermwidget.h"

//namespace Ui {
//class AdbShell;
//}

class AdbShell : public AbstractMDIChild
{
    Q_OBJECT
    
public:
    explicit AdbShell(QWidget *parent = 0);
    ~AdbShell();
    QTermWidget *getConsole();
    
private:
    QTermWidget *m_console;
};


#endif // ADBSHELL_H
