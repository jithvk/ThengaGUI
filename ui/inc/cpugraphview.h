#ifndef CPUGRAPHVIEW_H
#define CPUGRAPHVIEW_H

#include <QWidget>
#include "QGraphicsScene"
#include "QMutex"
#include "QTime"
#include "QPen"
#include "QTimer"
#include "adbhandler.h"


namespace Ui {
class CPUGraphView;
}

#define CPU_FREQ_DEFAULT_MAX  100
#define MAX_CPU_SUPPORTED 8
#define CPU_COMPONENT_INVALID -1

class CPUGraphView : public QWidget
{
    Q_OBJECT

public:
    explicit CPUGraphView(QWidget *parent = 0);
    ~CPUGraphView();

    void resizeEvent(QResizeEvent *event);
    void addPoint(long point, int component);
    void drawGraphBackground();
    void cleanup();
    void reset();
    void nextFrame(void *ptpObjectHandler);


    QString frequencyString(long freqValue);
    int selectedComponent() const;
    void setSelectedComponent(int selectedComponent);

public slots:
    void on_timer();
    void startTest();
    void saveGraph();

protected:
    bool initFrequencies();

private:
    Ui::CPUGraphView *ui;

    AdbHandler m_handler;


    QTimer *m_timer;
    QGraphicsScene *m_scene;



//    QThread *m_downloadThread;

    int prevPosition[MAX_CPU_SUPPORTED];
    int prevValue[MAX_CPU_SUPPORTED];
    QList<long> m_pointList[MAX_CPU_SUPPORTED];
    QGraphicsItem *m_currentFreqLabel[MAX_CPU_SUPPORTED];

    long m_maxFreq;
    long m_minFreq;

    int m_selectedComponent;



    QPen red;
    QPen green;
    QPen blue;
    QPen black;
    QPen gray;

    QMutex m_syncMutex;
    QTime m_elapsedTime;

    int framesInCurrentSecond; /* number of frames downloaded in current second, till now */
    int speedInCurrentSecond;
};

#endif // CPUGRAPHVIEW_H
