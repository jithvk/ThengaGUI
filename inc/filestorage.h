#ifndef FILESTORAGE_H
#define FILESTORAGE_H

#include <QObject>
#include "QString"
#include "stdint.h"


#define free_if_valid(a)   if(a != NULL)free(a)


struct StorageInfo {
    uint16_t StorageType;
    uint16_t FilesystemType;
    uint16_t AccessCapability;
    uint64_t MaxCapability;
    uint64_t FreeSpaceInBytes;
    char 	*StorageDescription;
    char	*VolumeLabel;
};

class FILEStorage : public QObject
{
    Q_OBJECT
public:
    explicit FILEStorage(QObject *dummy = 0);

    uint32_t storageId;
    StorageInfo storageInfo;

    /* methods*/
    FILEStorage(const FILEStorage &dummy);
    ~FILEStorage();




    operator QString()
    {
        QString s;
        if(storageInfo.StorageDescription != 0)
            s.sprintf("%s(0x%X)",storageInfo.StorageDescription,storageId);
        else if(storageInfo.VolumeLabel != 0)
            s.sprintf("%s(0x%X)",storageInfo.VolumeLabel,storageId);
        else
            s.sprintf("noname(0x%X)",storageId);
        return s;
    }

    int getType()
    { return storageInfo.StorageType; }

    uint64_t getFreeSpace()
    { return storageInfo.FreeSpaceInBytes;}


    FILEStorage &operator=(const FILEStorage &);


};

#endif // FILESTORAGE_H
