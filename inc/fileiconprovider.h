#ifndef FILEICONPROVIDER_H
#define FILEICONPROVIDER_H

#include "QPixmapCache"
#include "QFileIconProvider"


class FileIconProvider
{
public:
    static FileIconProvider * instance();
//    static QIcon fileIcon(const QString &fileSuffix);
//    static QIcon dirIcon();

private:
    FileIconProvider() {}

private:
    static FileIconProvider *self;
    QPixmapCache iconCache;
    QFileIconProvider iconProvider;
};

#endif // FILEICONPROVIDER_H
