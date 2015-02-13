#include "filestorage.h"

FILEStorage::FILEStorage(QObject *parent) :
    QObject(parent)
{
    storageId = 0;
    memset(&storageInfo,0,sizeof(storageInfo));
}


FILEStorage::FILEStorage(const FILEStorage &dummy):
    QObject()
{
    storageId = dummy.storageId;
    storageInfo = dummy.storageInfo;
    if(storageInfo.VolumeLabel != 0)
        storageInfo.VolumeLabel = strdup(dummy.storageInfo.VolumeLabel);
    if(storageInfo.StorageDescription != 0)
        storageInfo.StorageDescription = strdup(dummy.storageInfo.StorageDescription);
}

FILEStorage::~FILEStorage()
{
    free_if_valid(storageInfo.VolumeLabel);
    free_if_valid( storageInfo.StorageDescription);
}

FILEStorage & FILEStorage::operator=(const FILEStorage &dummy)
{

    if(this != &dummy)
    {
        storageId = dummy.storageId;
        storageInfo = dummy.storageInfo;
        if(storageInfo.VolumeLabel != 0)
            storageInfo.VolumeLabel = strdup(dummy.storageInfo.VolumeLabel);
        if(storageInfo.StorageDescription != 0)
            storageInfo.StorageDescription = strdup(dummy.storageInfo.StorageDescription);
    }
    return *this;
}
