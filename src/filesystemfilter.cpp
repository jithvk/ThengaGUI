#include "filesystemfilter.h"
#include "direntry.h"


FileSystemFilter::FileSystemFilter(QObject *parent)
    :QSortFilterProxyModel(parent)
{
    m_filterType = NO_FILTER;
    m_currentLevel = NULL;

    setDynamicSortFilter(true);

}



FilterType FileSystemFilter::filterType() const
{
    return m_filterType;
}

void FileSystemFilter::setFilterType(const FilterType &filterType)
{
    m_filterType = filterType;
}


void FileSystemFilter::setLevel(DirEntry *level)
{
    m_currentLevel = level;

}



bool FileSystemFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{



    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);


    if(index.isValid() == false)
        return false;

    if(m_filterType == NO_FILTER )
        return true;


    DirEntry *parent = (DirEntry*)index.internalPointer();
    if(parent == NULL )
        return false;

    DirEntry *selection = (DirEntry*)parent->child(index.row());
    if(selection == NULL)
        return false;

    if(selection->getObjectId() == 0 ) /* root obj*/
        return true;

    switch (m_filterType) {
    case DIR_ONLY:
        return selection->isDir();
    case CURRENT_LEVEL_ONLY:
        if(m_currentLevel == NULL)
            return false;
        if(selection->parent() == m_currentLevel)
                return true;
    default:
        return false;
    }

//    return false;

}

bool FileSystemFilter::filterAcceptsColumn(int /*source_column*/, const QModelIndex &/*source_parent*/) const
{

    return true;

}
