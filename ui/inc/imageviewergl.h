#ifndef IMAGEVIEWERGL_H
#define IMAGEVIEWERGL_H

#include <QObject>
#include <QtOpenGL>
#include <QGLWidget>
#include <QOpenGLFunctions>




#define IMAGE_TILE_WIDTH  512
#define IMAGE_TILE_HEIGHT 512

class ImageViewerGL : public QGLWidget, protected QOpenGLFunctions
{

    Q_OBJECT

private:
     GLuint m_imageTexture;
     QImage *m_image;
     GLfloat zoom;

     QSize imageSize;
     QPoint lastClick;

     int numTiles,numTilesX,numTilesY;
     int focusX,focusY;
     int rotationAngle;
     GLuint *imageTextures;
     QVector<QImage> imageArray;
     QString m_title;

     bool m_hasInitialized;


#ifdef GL_ES_VERSION_2_0
     /*For shader program    */
     QOpenGLShaderProgram *program;
#endif


signals:
     void closed();

public:
    ImageViewerGL(QWidget *parent = NULL);
    ~ImageViewerGL();
    void LoadShader(QString vshader, QString fshader);
    void closeEvent(QCloseEvent *event);


    QImage copyImage()
    {
        if(m_image == NULL)
            return QImage();

        return QImage(*m_image);
    }

    void Left();
    void Right();

    void open(QImage);
    void zoomIn();
    void zoomOut();
    void save(QString pathName);

    void rotateRight();
    void rotateLeft();

    void setTitle(QString title)
    {
        m_title = title;
    }

    inline QSize getImageSize()
    {
        if(zoom <= 0 )
            return sizeHint();

        return QSize(imageSize.width()*zoom,imageSize.height()*zoom);
    }

    QSize sizeHint()
    {
        return QSize(imageSize.height(),imageSize.width());
    }

//    QSize minimumSizeHint()
//    {
//        if(zoom > 0)
//            return QSize(imageSize.height()*zoom,imageSize.width()*zoom);
//        else
//            return QSize(640,480);
//    }


protected:
    void initializeGL();
    void paintGL();
    void drawImage(int,int);
    void resizeGL(int width, int height);
    void destroyContents();
    void draw(GLvoid *vertexArray, GLvoid *texArray);
    void texturizeImage();

    void    mouseMoveEvent ( QMouseEvent * event );
    void    mousePressEvent ( QMouseEvent * event );
    void    wheelEvent ( QWheelEvent * event );



};

#endif // IMAGEVIEWERGL_H
