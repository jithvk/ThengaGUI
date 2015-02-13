#include "QString"
#include "QFile"
#include "QFileInfo"
#include "QDebug"
#include "sys/stat.h"
#include "QDir"
#include "errno.h"

extern "C" {
#include "file_sync_service.h"
#include "fdevent.h"
#include "adb.h"
#include "adb_client.h"

#include "sysdeps.h"

//int  adb_close(int  fd);
//int  adb_read(int  fd, void* buf, int len);
int sync_readmode(int fd, const char *path, unsigned *mode);
void sync_quit(int fd);

}
#include "stdint.h"

#define ENABLE_PROGRESS 1


struct syncsendbuf {
    unsigned id;
    unsigned size;
    char data[SYNC_DATA_MAX];
};

void show_progress(qlonglong completed, QString path )
{

}


static int write_data_file(int fd, QString path, syncsendbuf *sbuf)
{
    int err = 0;
    QFile sourceFile(path);

#ifdef ENABLE_PROGRESS
    qlonglong size = 0;
    qlonglong completedTransefer = 0;
#endif

    if(sourceFile.open(QFile::ReadOnly) == false) {
//        fprintf(stderr,"cannot open '%s': %s\n", path, strerror(errno));
        qDebug()<<"Cannot open file: "<<path;
        return -1;
    }

#ifdef ENABLE_PROGRESS
    size = sourceFile.size();
#endif


    sbuf->id = ID_DATA;

    for(;;) {
        int ret;

        ret = sourceFile.read(sbuf->data, SYNC_DATA_MAX);

        if(!ret)
            break;

        if(ret < 0) {
            if(errno == EINTR)
                continue;
            qDebug()<<"Cannot read file: "<<path;
            break;
        }

        sbuf->size = htoll(ret);
        if(writex(fd, sbuf, sizeof(unsigned) * 2 + ret)){
            err = -1;
            break;
        }

#ifdef ENABLE_PROGRESS
        completedTransefer += ret;
        if(size > SYNC_DATA_MAX)
            show_progress((completedTransefer*100)/size,path);
#endif

    }


    return err;
}



static int sync_send(int fd, const char *lpath, const char *rpath,
                     unsigned mtime, mode_t mode)
{
    syncmsg msg;
    int len, r;
    syncsendbuf sbuf;// = &send_buffer;
    char* file_buffer = NULL;    
    char tmp[64];


    len = strlen(rpath);
    if(len > 1024) return -1;

    snprintf(tmp, sizeof(tmp), ",%d", mode);
    r = strlen(tmp);


    msg.req.id = ID_SEND;
    msg.req.namelen = htoll(len + r);

    if(writex(fd, &msg.req, sizeof(msg.req)) ||
       writex(fd, rpath, len) || writex(fd, tmp, r)) {
        free(file_buffer);
        return -1;
    }

    if (S_ISREG(mode))
        write_data_file(fd, lpath, &sbuf);
    else
        return -1;

    msg.data.id = ID_DONE;
    msg.data.size = htoll(mtime);
    if(writex(fd, &msg.data, sizeof(msg.data)))
        return -1;

    if(readx(fd, &msg.status, sizeof(msg.status)))
        return -1;

    if(msg.status.id != ID_OKAY) {
        if(msg.status.id == ID_FAIL) {
            len = ltohl(msg.status.msglen);
            if(len > 256) len = 256;
            if(readx(fd, sbuf.data, len)) {
                return -1;
            }
            sbuf.data[len] = 0;
        } else
            strcpy(sbuf.data, "unknown reason");

        return -1;
    }

    return 0;

}



/* Only does single file, no recursion*/
int sync_push(QString lpath, QString rpath)
{
    struct stat st;
//    unsigned mode;
    int fd;
//    QFileInfo sourcInfo(lpath);

    fd = _adb_connect("sync:");
    if(fd < 0) {
        qDebug()<<"Cant connect to device :"<<adb_error();
        return 1;
    }

    if(stat(lpath.toLocal8Bit().data(), &st)) {
//        fprintf(stderr,"cannot stat '%s': %s\n", lpath, strerror(errno));
        qDebug()<<"Cant get file stats "<<lpath;
        sync_quit(fd);
        adb_close(fd);
        return 1;
    }

    if(sync_send(fd, lpath.toLocal8Bit().data(), rpath.toLocal8Bit().data(),
                 st.st_mtime, st.st_mode)) {
        adb_close(fd);
        return 1;
    }

    sync_quit(fd);
    adb_close(fd);
    return 0;
}


#if 0

bool remove_file_or_Dir(const QString &dirName)
{
    bool result = true;
    QFileInfo info(dirName);
    if(info.exists() == false)
        return true;

    if (info.isDir()) {
        QDir dir(info.absoluteDir());
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System |
                                                    QDir::Hidden  | QDir::AllDirs |
                                                    QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = remove_file_or_Dir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    else
        result = QFile::remove(info.absoluteFilePath());

    return result;
}
#endif



/* should give excat name of lpath and rpath
 * lPath should be valid, ie creating the file lPath should be possible
*/
int sync_pull(QString rPath, QString lPath)
{

    syncmsg msg;
    QFile file;

    int len;
    char buffer[SYNC_DATA_MAX];// = send_buffer.data;
    unsigned id;

    int fd;

    fd = _adb_connect("sync:");

    if(fd < 0) {
        qDebug()<<"Cant connect to device :"<<adb_error();
        return 1;
    }

    len = rPath.length();
    if(len > 1024) return -1;

    msg.req.id = ID_RECV;
    msg.req.namelen = htoll(len);
    if(writex(fd, &msg.req, sizeof(msg.req)) ||
       writex(fd, rPath.toLocal8Bit().data(), len)) {
        return -1;
    }

    if(readx(fd, &msg.data, sizeof(msg.data))) {
        return -1;
    }
    id = msg.data.id;

    if((id == ID_DATA) || (id == ID_DONE)) {
        QFile::remove(lPath);

        file.setFileName(lPath);
        if(!file.open(QFile::WriteOnly))
        {
            qDebug()<<"cannot create "<<lPath<<" :"<< strerror(errno);
            return -1;
        }
        goto handle_data;
    } else {
        goto remote_error;
    }

    for(;;) {
        if(readx(fd, &msg.data, sizeof(msg.data))) {
            return -1;
        }
        id = msg.data.id;

    handle_data:
        len = ltohl(msg.data.size);
        if(id == ID_DONE) break;
        if(id != ID_DATA) goto remote_error;
        if(len > SYNC_DATA_MAX) {
            qDebug()<<"Data Overrun ";
            file.close();
            return -1;
        }

        if(readx(fd, buffer, len)) {
            file.close();
            return -1;
        }

//        if(writex(lfd, buffer, len)) {

        if(file.write(buffer,len) != len)
        {
            qDebug()<<"cannot write "<<rPath<<" :"<< strerror(errno);
            file.close();
            return -1;
        }

//        total_bytes += len;
    }

    file.close();
    sync_quit(fd);
    return 0;

remote_error:

//    adb_unlink(lPath.toLocal8Bit().data());
    QFile::remove(lPath);

    if(id == ID_FAIL) {
        len = ltohl(msg.data.size);
        if(len > 256) len = 256;
        if(readx(fd, buffer, len)) {
            return -1;
        }
        buffer[len] = 0;
    } else {
        memcpy(buffer, &id, 4);
        buffer[4] = 0;
//        strcpy(buffer,"unknown reason");
    }
    qDebug()<<"Failed to copy "<<rPath<<" to "<<lPath;
    return 0;
}


