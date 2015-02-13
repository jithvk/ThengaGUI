#include "fileobject.h"

//#define free_if_valid(a)   if(a != NULL)free(a)


FileObject::FileObject(QObject *parent) :
    QObject(parent)
{
    objectID  = 0;
    m_fileSize = 0;
    m_associationType = 0;
    m_parentObject = 0;

//    memset(&objectInfo,0,sizeof(objectInfo));
}



FileObject::FileObject(const FileObject &dummy):
    QObject()
{
    /* Copy constructor*/
    objectID = dummy.objectID;
    m_fileSize = dummy.m_fileSize;
    m_associationType = dummy.m_associationType;
    m_parentObject = dummy.m_parentObject;
    m_fileName= dummy.m_fileName;

//    memcpy(&objectInfo,&dummy.objectInfo,sizeof(objectInfo));
//    if(objectInfo.Filename != 0)
//       objectInfo.Filename = strdup(dummy.objectInfo.Filename);

}

FileObject::~FileObject()
{
//    free_if_valid(objectInfo.Filename);
////    free_if_valid( storageInfo.StorageDescription);
}

FileObject & FileObject::operator=(const FileObject &dummy)
{
    if(this != &dummy)
    {
        objectID = dummy.objectID;
        m_fileSize = dummy.m_fileSize;
        m_associationType = dummy.m_associationType;
        m_parentObject = dummy.m_parentObject;
        m_fileName= dummy.m_fileName;
    }
    return *this;
}
