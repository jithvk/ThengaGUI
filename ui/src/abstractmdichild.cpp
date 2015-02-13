#include "ui/inc/abstractmdichild.h"
#include "QMdiSubWindow"

AbstractMDIChild::AbstractMDIChild(QWidget *parent) :
    QWidget(parent)
{
    baseMDIArea = NULL;
    subWindow = NULL;
    setWindowTitle("");
    setMinimumSize(400,200);

}

AbstractMDIChild::~AbstractMDIChild()
{
    baseMDIArea = NULL;
    subWindow = NULL;
}

void AbstractMDIChild::addToMDIArea(QMdiArea *base)
{
    baseMDIArea = base;
    QObject::connect(this,SIGNAL(finished()),
                     this,SLOT(removeWindow()));


    subWindow = baseMDIArea->addSubWindow(this);//,Qt::FramelessWindowHint);
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
}


void AbstractMDIChild::removeWindow()
{
    if(baseMDIArea && subWindow )
    {
        baseMDIArea->removeSubWindow(qobject_cast<QMdiSubWindow *>(subWindow));
//        subWindow->close();
        subWindow->deleteLater();
    }
    deleteLater();
    baseMDIArea = NULL;
}

