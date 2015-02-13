#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QWidget>
#include "imageviewergl.h"

namespace Ui {
    class ImageView;
}

class ImageView : public QDialog
{
    Q_OBJECT

public:
    explicit ImageView(QImage img,QWidget *parent = 0);
    ~ImageView();

    virtual void resizeEvent(QResizeEvent *);

private:
    Ui::ImageView *ui;
    ImageViewerGL *m_image;
};

#endif // IMAGEVIEW_H
