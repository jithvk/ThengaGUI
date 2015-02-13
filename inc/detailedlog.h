#ifndef DETAILEDLOG_H
#define DETAILEDLOG_H

#ifndef __cplusplus
void gp_log(int level,char* domain,char *format,...);
void gp_log_data(char *name,char *data,int length);
#endif

#ifdef __cplusplus
#include "QObject"
#include "QStack"

typedef enum {
    ADB_LOG_ERROR = 0,	/**< \brief Log message is an error infomation. */
    ADB_LOG_VERBOSE = 1,	/**< \brief Log message is an verbose debug infomation. */
    ADB_LOG_DEBUG = 2,	/**< \brief Log message is an debug infomation. */
    ADB_LOG_DATA = 3		/**< \brief Log message is a data hex dump. */
} ADB_LogLevel;


extern "C" {
    void gp_log(int level, const char *domain, const char *format,...);
    void gp_log_data(const char *name, char *data, int length);
}


class DetailedLog
{
private:
    DetailedLog(QObject *);
    static DetailedLog *m_instance;
    QObject *m_parent;
    QStack<int> m_logStack;

    int m_currentLogLevel;
public:
    static DetailedLog *Init(QObject *parent);
    static void log(int level, const char *module, const char *message);
    static void log_data(const char *name, char *data, int length);
    static void showMessage(char * message);
    static void showMessage(QString message);
    static int getLogLevel();

    static DetailedLog * getInstance()
    { return m_instance; }

    void setLogLevel(int level);




    inline static bool isHtmlAvailable()
    {
        return (m_instance!=NULL && m_instance->m_parent!=NULL);
    }

    static void Push(int newLogLevel);
    static void Pop();

};

#endif

#endif // DETAILEDLOG_H
