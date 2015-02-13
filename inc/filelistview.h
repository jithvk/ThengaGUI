#ifndef FILELISTVIEW_H
#define FILELISTVIEW_H

#include <QListView>
#include "QMimeData"



class DragDropMimeData : public QMimeData
{
    Q_OBJECT

public:
    DragDropMimeData();

    QStringList formats() const;
    QString m_fileName;

protected:
    QVariant retrieveData(const QString &format,
                          QVariant::Type preferredType) const;

private:
    QStringList m_supportedFormats;


    // QMimeData interface
public:
    bool hasFormat(const QString &mimetype) const;
};

class FileListView : public QListView
{
    Q_OBJECT

public:
    explicit FileListView(QWidget *parent = 0);

    void performDragOut();
signals:

public slots:

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    QPoint m_startPos;

};

#endif // FILELISTVIEW_H
