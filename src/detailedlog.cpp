#include "detailedlog.h"
#include "QDebug"
#include "stdarg.h"
#include "stdio.h"
#include "mainwindow.h"

DetailedLog::DetailedLog(QObject *parent)
{
    m_parent = parent;
    m_currentLogLevel = ADB_LOG_ERROR;
//    m_logStack.clear();
//    m_logStack  = new QStack<int>();
}

DetailedLog *DetailedLog::m_instance = NULL;

DetailedLog *DetailedLog::Init(QObject *parent)
{
    if(m_instance != NULL)
        delete m_instance;

    m_instance = new DetailedLog(parent);
    return m_instance;
}

int DetailedLog::getLogLevel()
{
    /* only log erros by default*/
    if(m_instance == NULL)
        return ADB_LOG_ERROR;

    return m_instance->m_currentLogLevel;

}

void DetailedLog::Push(int newLogLevel)
{

    if(getInstance() == NULL)
        return;

     getInstance()->m_logStack.push(getInstance()->m_currentLogLevel);

    if(getInstance()->m_currentLogLevel > newLogLevel)
    {
//        showMessage(QString("<Switching log level..>\n"));
        getInstance()->m_currentLogLevel = newLogLevel;
    }

}


void DetailedLog::Pop()
{

    if(getInstance() == NULL)
        return;

    if( getInstance()->m_logStack.isEmpty())
        return;

//    showMessage(QString("<Switching back log level..>\n"));
     getInstance()->m_currentLogLevel =  getInstance()->m_logStack.pop();

}


void DetailedLog::showMessage(char *message)
{


    if(isHtmlAvailable() == false)
    {
        qDebug(message);
        return;
    }
    QString textMessage = QString::fromLocal8Bit(message);

    MainWindow *parentWindow = (MainWindow*)m_instance->m_parent;
    parentWindow->AppendHTMLText(textMessage);
}


void DetailedLog::showMessage(QString message)
{


    if(isHtmlAvailable() == false)
    {
        qDebug(message.toLocal8Bit().data());
        return;
    }


    MainWindow *parentWindow = (MainWindow*)m_instance->m_parent;
    parentWindow->AppendHTMLText(message);
}

void DetailedLog::setLogLevel(int level)
{
    this->m_currentLogLevel = level;
}





void gp_log(int level,const char* domain,const char *format,...)
{
    if(level > DetailedLog::getLogLevel() )
        return;
    char message[1024];
    va_list va;
    va_start(va,format);
    vsprintf(message,format,va);
    va_end(va);
    DetailedLog::log(level,domain,message);
}


void gp_log_data(const char *name,char *data,int length)
{
    if(ADB_LOG_DATA > DetailedLog::getLogLevel() )
        return;
    DetailedLog::log_data(name,data,length);
}

void DetailedLog::log(int level,const char * module,const char * message)
{
    char tmp[1024];
    if(isHtmlAvailable())
    {
        switch (level)
        {
        default:
        case ADB_LOG_VERBOSE:
          sprintf(tmp,"<font color = \"#3399ff\"><b>%s </b>: %s</font>\n",module,message);
            break;
        case ADB_LOG_DEBUG:
            sprintf(tmp,"<font color = \"green\"><b>%s </b>: %s</font>\n",module,message);
            break;
        case ADB_LOG_ERROR:
            sprintf(tmp,"<font color = \"red\"><b>%s </b>: %s</font>\n",module,message);
                break;

        }

    }
    else
        sprintf(tmp,"%s :%s\n",module,message);

    showMessage(tmp);
}

char * toHex(unsigned short c)
{
    static char tmp[5];
    sprintf(tmp,"%02X ",c & 0xFF);
    return tmp;
}

void DetailedLog::log_data(const char *name,char *data,int length)
{
    if(isHtmlAvailable() == true)
        showMessage(QString("<font color = \"green\"><b>[%1 ] %2 bytes</b>").arg(name).arg(length));
    else
        showMessage(QString("[DEBUG]: %1").arg(name));
    QString message = "";
    int i=0,j=0;
    for(i =0;i<length;i+=16)
    for(j=0;j<16;j++)
    {
        if(i+j>= length)
            break;
        if(j == 8 )
            message.append("  |  ");
        if((j == 0))
            message.append("\n");

        message.append(toHex(data[i+j]));

        if(j == 15 || (i+j == length - 1) )
        {
            message.append(" | ");
            for(int k = 0;k<=j;k++)
                if(isalnum((data[i+k])))
                 message.append((char)(data[i+k]));
              else
                  message.append((char)'.');

        }
    }
    message.append("</font>\n");

    showMessage(message);
}




//void gp_set_logFunction(GPLogFunc func);
