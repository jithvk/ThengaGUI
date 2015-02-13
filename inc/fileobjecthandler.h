#ifndef FILEOBJECTHANDLER_H
#define FILEOBJECTHANDLER_H
#include <stdint.h>


/* memory increment */
#define  MEMORY_INCRIMENT  (1*1024*1024)

#ifndef NO_ERROR
#define NO_ERROR  0
#endif
#define GENERIC_ERROR (-1)


#ifdef __cplusplus
#include "QFile"
#endif


class FILEObjectHandler : public QObject
{

    Q_OBJECT

public:
    enum HandlerType
    {
        MEMORY,
        FILE
    };



    /* callback function for status indication ,
    -ve return means error, will cancel the operation*/
    typedef int (*status_callback)(void* data,long percentageComplete);
private:
    HandlerType m_type;
    /* data pointer for memory reciever*/
    void *m_data;

    /* file descriptor for file reciever*/
    QFile *m_file;

    /* current offset for read or write*/
    unsigned long m_currentOffset;

    /* Length of buffer allocated for m_data, used only for memory reciever*/
    unsigned long m_bufferLength;

    /* file size of target*/
    unsigned long m_targetFileSize;

    /* read the data from buffer or file*/
    void internal_readData(char *data, unsigned int *length);

    /* write the data to buffer or file*/
    void internal_writeData(const char *data, unsigned int *length);


    /* for progress callback */
    void *m_callbackData;
    status_callback fn_StatusCallback;

    bool cancelled;



public:
    FILEObjectHandler(HandlerType type=MEMORY);
    ~FILEObjectHandler();

    /*TargetFile: give the file name to save the object
    Returns -1 if file cant be open for writing
    Note: This will truncate any existing file of same name
    */
    int SetTargetFile(QString fileName);

    int SetSourceFile(QString fileName);

    /*TargetSize: an approximate size. No issue even if actual object is bigger than this.
      Applicable only for memory Objects
      Returs: 0 if success
              -1 if memory allocation fails      */
    int SetTargetSize(unsigned long targetSize);

    inline unsigned long getObjectSize()
    {
        return m_currentOffset;
    }
    inline unsigned long getTargetSize()
    {
        return m_targetFileSize;
    }

    inline void * getData()
    {
        return m_data;
    }

    inline bool isCancelled()
    {
        return cancelled;
    }

    void SetProgressNotification(void* data,status_callback callback);


    static uint16_t ReadData(void*priv,
                                            unsigned long wantlen,
                                            unsigned char *data, unsigned long *gotlen);

    static uint16_t WriteData(void*priv,
                                            unsigned long sendlen,
                                            unsigned char *data, unsigned long *writelen);


public slots:
    void cancel();
};

#endif // FILEOBJECTHANDLER_H
