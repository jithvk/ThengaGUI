#ifndef FILESYSTEMFILTER_H
#define FILESYSTEMFILTER_H

#include <QSortFilterProxyModel>
#include "filesystemmodel.h"

enum FilterType
{
    DIR_ONLY,
    CURRENT_LEVEL_ONLY,
    NO_FILTER
};

class FileSystemFilter : public QSortFilterProxyModel
{
    Q_OBJECT

private:
    FilterType m_filterType;
    DirEntry *m_currentLevel;

public:
    FileSystemFilter(QObject *parent = NULL);


    FilterType filterType() const;
    void setFilterType(const FilterType &filterType);
    void setLevel(DirEntry *level);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool filterAcceptsColumn(int, const QModelIndex &) const;
};

#endif // FILESYSTEMFILTER_H
