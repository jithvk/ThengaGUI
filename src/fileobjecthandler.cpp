
#include "stdlib.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <unistd.h>
#include <string.h>
#include "fileobjecthandler.h"
#include "detailedlog.h"
#include "QApplication"
#include "QFile"

FILEObjectHandler::FILEObjectHandler(HandlerType type)
    : QObject(NULL)
{
    m_type = type;
    m_targetFileSize = 0;
    m_data = NULL;
    m_file = 0;
    m_currentOffset = 0;
    m_bufferLength= 0;
    cancelled = false;

    m_callbackData = NULL;
    fn_StatusCallback = NULL;
}

FILEObjectHandler::~FILEObjectHandler()
{
    switch(m_type)
    {
    case FILE:
        if(m_file)
        {
            m_file->close();
            delete m_file;
        }
        break;

    case MEMORY:
        if(m_data)
            free(m_data);
        break;
    }

    m_targetFileSize = 0;
    m_data = NULL;
    m_file = 0;
    m_currentOffset = 0;
    m_bufferLength= 0;
    m_callbackData = NULL;
    fn_StatusCallback = NULL;
}

/* fix for linux where O_BINARY is not defined*/
#ifndef O_BINARY
#define O_BINARY 0
#endif

int FILEObjectHandler::SetTargetFile(QString fileName)
{
    if(m_type != FILE)
        return GENERIC_ERROR;


    if(m_file)
        delete m_file;


    m_file = new QFile(fileName);
    m_file->open(QIODevice::WriteOnly | QIODevice::Truncate);


    m_currentOffset = 0;

//    m_file = open(fileName,O_CREAT|O_RDWR|O_TRUNC|O_BINARY,S_IRUSR|S_IWUSR );
    if(!m_file->isWritable())
        return GENERIC_ERROR;

    return NO_ERROR;
}

int FILEObjectHandler::SetSourceFile(QString fileName)
{
    if(m_type != FILE)
        return GENERIC_ERROR;


    if(m_file)
        delete m_file;

    m_file = new QFile(fileName);
    m_file->open(QIODevice::ReadOnly) ;

    m_currentOffset = 0;
//    m_file = open(fileName,O_RDONLY|O_BINARY,S_IRUSR );
    if(!m_file->isReadable())
        return GENERIC_ERROR;


    m_targetFileSize = m_file->size();

    return NO_ERROR;
}

int FILEObjectHandler::SetTargetSize(unsigned long targetSize)
{

    switch(m_type)
    {

    case MEMORY:
        m_data = malloc(targetSize);
        if(m_data == NULL)
            return GENERIC_ERROR;
        m_bufferLength = targetSize;

        /* fallthrough*/
    case FILE:
       m_targetFileSize = targetSize;

    }
    return NO_ERROR;
}

void FILEObjectHandler::internal_writeData(const char *data, unsigned int *length)
{
    int actualWriteLength = 0;
    int allocationLength = 0;

    if(cancelled)
    {
        *length = 0;
        return;
    }

    if(m_type == FILE)
    {

        m_file->seek(m_currentOffset);

        actualWriteLength = m_file->write(data,*length);

        *length = actualWriteLength;
        m_currentOffset+= actualWriteLength;
        gp_log(ADB_LOG_DEBUG,"FileObjectHandler","Wrote %d to file, Current Offset = %d",
               actualWriteLength,m_currentOffset);
    }
    else if(m_type == MEMORY)
    {
        if (m_currentOffset + *length > m_bufferLength) {
            allocationLength = (m_currentOffset + *length - m_bufferLength)>MEMORY_INCRIMENT?
                                       m_currentOffset + *length - m_bufferLength:MEMORY_INCRIMENT;
                void *newData = realloc (m_data, m_bufferLength + allocationLength);
                if (!newData)
                {
                    *length = 0;
                    gp_log(ADB_LOG_ERROR,"FileObjectHandler",
                           "Unable to allocate enough memory" );
                    return;
                }

                m_data = newData;
                m_bufferLength+=allocationLength;

        }
        memcpy(((char*)m_data+m_currentOffset),data,*length);
        m_currentOffset += *length;
    }

    return;

}

void FILEObjectHandler::internal_readData(char *data, unsigned int *length)
{
    int actualReadLength = 0;

    if(cancelled)
    {
        *length = 0;
        return;
    }

    if(m_type == FILE)
    {
        m_file->seek(m_currentOffset);
        actualReadLength = m_file->read(data,*length);

        *length = actualReadLength;
        m_currentOffset+= actualReadLength;
    }
    else if(m_type == MEMORY)
    {
        if (m_currentOffset + *length > m_bufferLength)
            actualReadLength = m_bufferLength - m_currentOffset;
        else
            actualReadLength = *length;

        memcpy(data,(char*)m_data + m_currentOffset,actualReadLength);
        *length = actualReadLength;
        m_currentOffset += *length;
    }
    return;
}



uint16_t FILEObjectHandler::WriteData(  void *priv,
                                            unsigned long sendlen,
                                            unsigned char *data,
                                            unsigned long *writelen)
{
    FILEObjectHandler *objInstance = (FILEObjectHandler*)priv;
    unsigned int length = sendlen;

    objInstance->internal_writeData((char * )data,&length);

    *writelen = length;

    if(objInstance->fn_StatusCallback &&
            objInstance->m_callbackData && length)
    {
        objInstance->fn_StatusCallback(objInstance->m_callbackData,
                                       objInstance->m_currentOffset);        
    }

    qApp->processEvents();

    if( length != 0)
        return NO_ERROR;

    return GENERIC_ERROR;

}

uint16_t FILEObjectHandler::ReadData( void *priv,
                                           unsigned long wantlen,
                                           unsigned char *data,
                                           unsigned long *gotlen)
{
    FILEObjectHandler *objInstance = (FILEObjectHandler*)priv;
    unsigned int length = wantlen;

    objInstance->internal_readData((char * )data,&length);

    *gotlen = length;

    qApp->processEvents();

    if( length!= 0)
        return NO_ERROR;

    return GENERIC_ERROR;
}


void FILEObjectHandler::cancel()
{
    gp_log(ADB_LOG_VERBOSE,"FileObjectHandler","Object Transfer cancelled");
    cancelled = true;
}

void FILEObjectHandler::SetProgressNotification(void *data, status_callback callback)
{
    m_callbackData = data;
    fn_StatusCallback = callback;
}

