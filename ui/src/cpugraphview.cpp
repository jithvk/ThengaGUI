#include "cpugraphview.h"
#include "ui_cpugraphview.h"
#include "QPaintEvent"
#include "QFileDialog"
#include "QThread"
#include "QGraphicsItem"
#include "QGraphicsTextItem"

#define delete_if_valid(x) {if(x!=NULL)delete(x);x=NULL;}

CPUGraphView::CPUGraphView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CPUGraphView)
{
    ui->setupUi(this);


    m_scene = new QGraphicsScene(0,0,width(),120,this);
    reset();


    ui->graphicsView->setScene(m_scene);

    m_timer = new QTimer(this);

    connect(m_timer,SIGNAL(timeout()),this,SLOT(on_timer()));

    m_maxFreq = CPU_FREQ_DEFAULT_MAX;
    m_minFreq = 0;   

    m_selectedComponent = CPU_COMPONENT_INVALID;


}


void CPUGraphView::cleanup()
{


    QMutexLocker m(&m_syncMutex);

#if 0
    /* kill the thread first*/
    if(m_downloadThread != NULL)
    {
        if(m_downloadThread->isRunning())
        {
            m_downloadThread->quit();
            m_downloadThread->wait();
        }
        delete m_downloadThread;
    }
    m_downloadThread = NULL;
#endif




    m_scene->clear();

    m_timer->stop();

    delete_if_valid(m_scene);
    delete_if_valid(ui);
    delete_if_valid( m_timer);

}

CPUGraphView::~CPUGraphView()
{
    cleanup();
}



void CPUGraphView::on_timer()
{
    QMutexLocker m(&m_syncMutex);

    if(m_maxFreq == CPU_FREQ_DEFAULT_MAX)
    {
        if(initFrequencies() == false)
            return;
        else
            drawGraphBackground();
    }

    /* No CPU is selected, all cpus to be measured*/
    if(selectedComponent() == CPU_COMPONENT_INVALID)
    {
        QList<long> speeds =m_handler.getCpuSpeeds();
        if(speeds.isEmpty())
        {
            m_timer->stop();
            return;
        }

        for(int component =0;component<speeds.size();component++)
            addPoint(speeds.at(component),component);
    }
    else
    {
        long speed = m_handler.getCpuSpeed(selectedComponent());
        addPoint(speed,selectedComponent());
    }

    qApp->processEvents();
    m_elapsedTime.restart();


}


void CPUGraphView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);


    QList<long> newPointList[MAX_CPU_SUPPORTED];

    for(int component=0;component<MAX_CPU_SUPPORTED;component++)
        newPointList[component]= m_pointList[component];

    reset();
    drawGraphBackground();

    for(int component=0;component<MAX_CPU_SUPPORTED;component++)
    {
        if(newPointList[component].isEmpty() == false)
        {
            for(int i=0;i<newPointList[component].size();i++)
                addPoint(newPointList[component].at(i),component);
        }
    }

}

void CPUGraphView::addPoint(long point,int component)
{
    if(point < 0)
        point = 0;

    int mx = ui->graphicsView->width()-20;
    int my = ui->graphicsView->height()-50;

    int value = (point*my*1.0)/(1.1*m_maxFreq); /* Max is 110%*/
//    QRect rect = QRect(0,0, mx,my );

    int deltaX = 20;
    const int startX = 40;

    if(prevPosition[component] +startX > mx)
    {
        drawGraphBackground();
        prevPosition[component] = 0;
        m_pointList[component].clear();
    }
    value = value>my?my:value;
    value = my- value;

    if(prevValue[component] == 0)
        prevValue[component] = my;


    QPen penToUse;
    switch(component)
    {
    case 0:
        penToUse=red;
        break;
    case 1:
        penToUse=blue;
        break;
    case 3:
        penToUse = green;
        break;

    default:
        penToUse = black;
    }

    penToUse.setWidth(2);
    int px,py,cx,cy;
    px = prevPosition[component] +component*2 +startX;
    py = prevValue[component]-component*2;
    cx = prevPosition[component]+deltaX + component*2 +startX;
    cy = value -component*2;

    m_scene->addLine(px,py,cx,py,penToUse);
    m_scene->addLine(cx,py,cx,cy,penToUse);


//    QString valueMessage= frequencyString(point);

//    if(m_currentFreqLabel[component] != NULL)
//    {
//        m_scene->removeItem(m_currentFreqLabel[component]);
//        delete m_currentFreqLabel[component];
//        m_currentFreqLabel[component]= NULL;
//    }

//    QGraphicsItem * txt = (QGraphicsItem* )m_scene->addText(valueMessage);
//    txt->setPos(cx,cy);
//    m_currentFreqLabel[component] = txt;

    prevPosition[component]+=deltaX;
    prevValue[component] = value;
    m_pointList[component].append(point);
}

bool CPUGraphView::initFrequencies()
{
    if(m_handler.isDeviceConnected() == true)
    {
        QList<long> list=m_handler.getCpuMaxSpeeds();
        for(int i = 0;i<list.size();i++)
        {
            if(m_maxFreq < list.at(i))
                m_maxFreq = list.at(i);
            /* we can show upto 110% of max freq*/
        }

        return m_maxFreq > CPU_FREQ_DEFAULT_MAX;
    }
    else
        return false;

}
int CPUGraphView::selectedComponent() const
{
    return m_selectedComponent;
}

void CPUGraphView::setSelectedComponent(int selectedComponent)
{
    m_selectedComponent = selectedComponent;
}



QString CPUGraphView::frequencyString(long freqValue)
{
    QString valueMessage;

    if(freqValue >1024*1025*1024)
        valueMessage = QString().setNum((float)(freqValue/(1024.0*1024.0*1024.0)),'g',2)+"THz";
    else if(freqValue > 1024*1024)
        valueMessage = QString().setNum((float)(freqValue/(1024.0*1024.0)),'n',2)+"GHz";
    else if(freqValue > 1024)
        valueMessage = QString().setNum((float)(freqValue/(1024.0)),'n',0)+"MHz";
    else
        valueMessage = QString().setNum((float)(freqValue),'n',0)+"KHz";

    return valueMessage;

}

void CPUGraphView::drawGraphBackground()
{

    if(m_maxFreq == CPU_FREQ_DEFAULT_MAX)
    {
        if(initFrequencies() == false)
            return;
    }

    int mx = ui->graphicsView->width()-20;
    int my = ui->graphicsView->height()-50;
    QRect rect = QRect(0,0, mx,my );

    int startX = 40;

    float percentageDy = my/110.0;

    m_scene->clear();

    /* need atleast 100pix height*/

    int gridDelta = 10;

    if(percentageDy < 1)
    {
        gridDelta = 50;
    }

    int graphGridSize = gridDelta*percentageDy;


    m_scene->setSceneRect(rect);


    int i,j;
    int percentValue = 0;
    long freqValue = 0;

    QString valueMessage;


    for(j=my;j>0;j-=graphGridSize)
    {
        m_scene->addLine(startX,j,mx,j,gray);

        if((my-j)%2*graphGridSize != 0 || my-j == 0)
            continue;

        percentValue = gridDelta*(my-j)/graphGridSize;
        freqValue = percentValue*m_maxFreq/100;

        valueMessage  = frequencyString(freqValue);


        m_scene->addText(valueMessage)->setPos(-10,j-10);
    }

    m_scene->addText("Frequency")->setPos(0,-25);


    /* constant grid size for horizondal axis*/
    graphGridSize = 20;

    for(i=0;i<mx-startX-graphGridSize;i+=graphGridSize)
    {
        m_scene->addLine(i+startX,0,i+startX,my,gray);

        if(i%(4*graphGridSize) != 0 )
            continue;

        valueMessage= QString("%1").arg(i/graphGridSize);
        m_scene->addText(valueMessage)->setPos(i-5+startX,my+5);
    }

    m_scene->addText("Seconds")->setPos(170,my+5);

    blue.setWidth(2);
    m_scene->addRect(QRect(startX,0,mx-startX,my),gray);

}

#if 0
Qt::white	3	White (#ffffff)
Qt::black	2	Black (#000000)
Qt::red	7	Red (#ff0000)
Qt::darkRed	13	Dark red (#800000)
Qt::green	8	Green (#00ff00)
Qt::darkGreen	14	Dark green (#008000)
Qt::blue	9	Blue (#0000ff)
Qt::darkBlue	15	Dark blue (#000080)
Qt::cyan	10	Cyan (#00ffff)
Qt::darkCyan	16	Dark cyan (#008080)
Qt::magenta	11	Magenta (#ff00ff)
Qt::darkMagenta	17	Dark magenta (#800080)
Qt::yellow	12	Yellow (#ffff00)
Qt::darkYellow	18	Dark yellow (#808000)
Qt::gray	5	Gray (#a0a0a4)
Qt::darkGray	4	Dark gray (#808080)
Qt::lightGray	6	Light gray (#c0c0c0)
Qt::transparent	19	a transparent black value (i.e., QColor(0, 0, 0, 0))
Qt::color0	0	0 pixel value (for bitmaps)
Qt::color1	1	1 pixel value (for bitmaps)
#endif

void CPUGraphView::reset()
{

    blue = QPen(QColor(Qt::blue));
    green = QPen(QColor(Qt::green));
    red = QPen(QColor(Qt::red));
    black = QPen(QColor(Qt::black));
    gray = QPen(QColor(Qt::lightGray));

    for(int component = 0;component<MAX_CPU_SUPPORTED;component++)
    {
        prevPosition[component] = 0;
        prevValue[component] = 0;
        m_pointList[component].clear();
        m_currentFreqLabel[component] = NULL;
    }

}





void CPUGraphView::nextFrame(void *ptpObjectHandler)
{

   qApp->processEvents();

   QMutexLocker m(&m_syncMutex);

   framesInCurrentSecond++;

}

void CPUGraphView::startTest()
{

    qApp->processEvents();

    m_timer->start(1000); /* start the per second timer*/
    m_elapsedTime.start();

}




void CPUGraphView::saveGraph()
{

    QImage img(1024,1024,QImage::Format_ARGB32_Premultiplied);

    if(m_scene == NULL)
        return;


    QString fileName = QFileDialog::getSaveFileName(this,"Save Graph",
                                                    QString(),"PNG Images (*.png);;");
    if(fileName.isEmpty())
        return;


    QPainter p(&img);

    ui->graphicsView->render(&p);

    p.end();

    img.save(fileName,"png");
}
