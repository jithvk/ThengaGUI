#ifndef ABSTRACTMDICHILD_H
#define ABSTRACTMDICHILD_H

#include <QObject>
#include "QDialog"
#include "QMdiArea"

class AbstractMDIChild : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractMDIChild(QWidget *parent = 0);
    ~AbstractMDIChild();

    void addToMDIArea(QMdiArea *base);
    void setParentSubWindow(QMdiSubWindow *sub);

    QMdiSubWindow *getSubWindow()
    {
        return subWindow;
    }


    
signals:
    void finished();
    
public slots:
    void removeWindow();

private:
    QMdiArea *baseMDIArea;
    QMdiSubWindow *subWindow;
    
};

#endif // ABSTRACTMDICHILD_H
