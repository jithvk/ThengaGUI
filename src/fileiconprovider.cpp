#include "inc/fileiconprovider.h"
#include "QFileIconProvider"
#include "QFileInfo"
#include "QPixmap"

FileIconProvider *FileIconProvider::self = 0;

FileIconProvider *FileIconProvider::instance()
{
    if(!self)
        self = new FileIconProvider();
    return self;
}


