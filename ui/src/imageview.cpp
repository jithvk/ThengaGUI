#include "imageview.h"
#include "ui_imageview.h"

ImageView::ImageView(QImage img,QWidget *parent) :    
    QDialog(parent),
    ui(new Ui::ImageView)
{
    ui->setupUi(this);

    m_image = new ImageViewerGL(this);
    m_image->open(img);
    m_image->resize(QWidget::size());
    m_image->setTitle(tr("Image Viewer"));
    setWindowFlags(windowFlags() | Qt::WindowMinMaxButtonsHint);

}

ImageView::~ImageView()
{
    delete ui;
    delete m_image;
}

void ImageView::resizeEvent(QResizeEvent *e)
{
    if(m_image != NULL)
        m_image->resize(QWidget::size());
    e->accept();

}
