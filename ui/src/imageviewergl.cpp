
#include "imageviewergl.h"
#include "iostream"

ImageViewerGL::ImageViewerGL(QWidget *parent)
: QGLWidget(parent)
{
    //    zoom =-1;
    m_image = NULL;
    imageTextures = NULL;
    rotationAngle = 0;
    m_title = QString();
    m_hasInitialized = false;

//    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

#ifdef GL_ES_VERSION_2_0
    program = NULL;
#endif


}


void ImageViewerGL::closeEvent(QCloseEvent *event)
{
    emit closed();
    event->accept();
}

ImageViewerGL::~ImageViewerGL()
{
    destroyContents();

#ifdef GL_ES_VERSION_2_0
    delete program;
    program = NULL;
#endif
    m_hasInitialized  = false;
}


void ImageViewerGL::destroyContents()
{
    //makeCurrent();
    if(imageArray.isEmpty() == false)
        imageArray.clear();

    if(m_image)
    {
        delete m_image;
        m_image = NULL;
    }
    if(imageTextures)
    {
        for(int i=0;i<numTilesX;i++)
            for(int j=0;j<numTilesY;j++)
                deleteTexture(imageTextures[i+j*numTilesX]);
        //            glDeleteTextures(1,&imageTextures[i+j*numTilesX]);
        delete [] imageTextures;
        imageTextures = NULL;
    }
    zoom = -1;
}


void ImageViewerGL::open(QImage image)
{
    if (image.isNull()) {
        qDebug("Invalid image");
        return ;
    }

    if(m_image != NULL)
    {
        GLfloat currentZoom = zoom;
        destroyContents();
        m_image = new QImage(image);
        texturizeImage();
        zoom = currentZoom;
    }
    else
    {
        m_image = new QImage(image);
        zoom  = -1;
    }
//    updateGeometry();
}


void ImageViewerGL::save(QString pathName)
{
    if(pathName.isEmpty())
    return;
    if(m_image->isNull() == false)
    m_image->save(pathName,0,100);
}





void ImageViewerGL::texturizeImage()
{

    if(m_hasInitialized == false)
        return;
    //initializeGL();

//    zoom=-1;
    QImage img;
    QTime startTime;
    if(m_image == NULL)
    {
        qDebug("Please open the widget first. Image is null");
        return ;
    }
    int i,j;
    int xLen, yLen;

    imageSize = m_image->size();
    numTilesY = qCeil(imageSize.height()/(float)IMAGE_TILE_HEIGHT);
    numTilesX = qCeil(imageSize.width()/(float)IMAGE_TILE_WIDTH);
    numTiles = (numTilesX)*(numTilesY);
    imageTextures = new GLuint[numTiles];

    /* measure the time taken to populate */
    startTime.start();

    glEnable(GL_TEXTURE_2D);


    for(j=0;j<numTilesY;j++)
      for(i=0;i<numTilesX;i++)
    {
        xLen = IMAGE_TILE_WIDTH;
        yLen = IMAGE_TILE_HEIGHT;

#ifndef GL_ES_VERSION_2_0
        if( imageSize.width() - i*IMAGE_TILE_WIDTH <IMAGE_TILE_WIDTH)
            xLen = imageSize.width() - i*IMAGE_TILE_WIDTH;
        if( imageSize.height() - j*IMAGE_TILE_HEIGHT <IMAGE_TILE_HEIGHT)
            yLen = imageSize.height() - j*IMAGE_TILE_HEIGHT;
#endif
        img=m_image->copy(i*IMAGE_TILE_WIDTH,j*IMAGE_TILE_HEIGHT,
        xLen,yLen);

        imageTextures[i+j*numTilesX]=bindTexture(img);
        // Set nearest filtering mode for texture minification
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        // Set bilinear filtering mode for texture magnification
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//        // Wrap texture coordinates by repeating
//        // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        imageArray.append(img);
    }

//    qDebug()<< "Time taken to populate: "<<startTime.elapsed();;
}


void ImageViewerGL::initializeGL()
{


    initializeOpenGLFunctions();

#ifdef GL_ES_VERSION_2_0
    LoadShader(":/shaders/vshader.glsl",":/shaders/fshader.glsl");
#endif

    m_hasInitialized = true;

}



void ImageViewerGL::paintGL()
{
    drawImage(width(),height());
}


void ImageViewerGL::draw(GLvoid * vertexArray,GLvoid * texArray)
{
#ifdef GL_ES_VERSION_2_0
    // Tell OpenGL programmable pipeline how to locate vertex position data
    int vertexLocation = program->attributeLocation("a_position");
    program->enableAttributeArray(vertexLocation);
    glVertexAttribPointer(vertexLocation,3,GL_SHORT,GL_FALSE,0,vertexArray);


    // Tell OpenGL programmable pipeline how to locate vertex texture coordinate data
    int texcoordLocation = program->attributeLocation("a_texcoord");
    program->enableAttributeArray(texcoordLocation);
    glVertexAttribPointer(texcoordLocation, 2, GL_SHORT, GL_FALSE,0,(const void *)texArray);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif
}



void ImageViewerGL::drawImage(int w, int h)
{
    if(w == 0 || h == 0)
    return;


    if(imageTextures == NULL)
        texturizeImage();

    /* check again if they were populated*/
    if(imageTextures == NULL)
        return;

//    zoom = 1.0;

    if(zoom < 0.0)
    {
        float widthRatio = w/(float)imageSize.width();
        float heightRatio = h/(float)imageSize.height();
        zoom=qMin(widthRatio,heightRatio);
        focusX = imageSize.width()/2;
        focusY = imageSize.height()/2;
    }

    int i,j;
    int zoomedImageHeight =  qCeil(zoom*imageSize.height());
    int zoomedImageWidth = qCeil(zoom*imageSize.width());
    int zoomedTileHeight = qCeil(zoom*IMAGE_TILE_HEIGHT);
    int zoomedTileWidth = qCeil(zoom*IMAGE_TILE_WIDTH);
    short tileX,tileY;
    short prevX = 0,prevY =  zoomedImageHeight;


    qglClearColor(Qt::lightGray);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SCISSOR_TEST);
    glViewport(0,0,w,h);
    glScissor(0,0,w,h);

#ifndef GL_ES_VERSION_2_0
    //     glClearColor( 0.5f, 0.5f, 0.5f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT );
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glShadeModel( GL_SMOOTH );
    glOrtho(0,w,0,h,-1,1);
    glTranslatef(w/2-focusX*zoom,h/2-focusY*zoom, 0.0 );
#endif


    /* calculations for images smaller than tile*/
    if(zoomedImageHeight <zoomedTileHeight)
        zoomedTileHeight = zoomedImageHeight;
    if(zoomedImageWidth < zoomedTileWidth)
        zoomedTileWidth = zoomedImageWidth;


#ifdef GL_ES_VERSION_2_0
    GLshort tex1[] = {
        0, 0,        // top left        (V2)
        0, 1,        // bottom left    (V1)
        1, 0,        // top right    (V4)
        1, 1        // bottom right    (V3)
    };
    QMatrix4x4 m;
    m.setToIdentity();
    m.ortho(0,w,0,h,-1,1);
    m.translate(w/2-focusX*zoom,h/2-focusY*zoom, 0.0 );
    m.rotate(rotationAngle , 0.0f, 0.0f, 1.0f);
    program->setUniformValue("mvp_matrix", m);

    program->setUniformValue("texture", 0);

#else
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(zoomedImageWidth/2,zoomedImageHeight/2,0.0);
    glRotatef(rotationAngle,0.0,0.0,1.0);
    glTranslatef(-zoomedImageWidth/2,-zoomedImageHeight/2,0.0);
#endif


    for(j=0;j < numTilesY;j++)
    {

        tileY = prevY ;
        prevY -=zoomedTileHeight;

        if(prevY < 0)
            prevY = 0;

        tileX = 0;
        prevX = zoomedTileWidth;
        for(i=0;i<numTilesX;i++)
        {



#ifdef GL_ES_VERSION_2_0

            GLshort vtx1[] = {
                tileX, prevY, 0,
                tileX,tileY, 0,
                prevX, prevY, 0,
                prevX, tileY, 0
            };

            glBindTexture(GL_TEXTURE_2D, imageTextures[i+j*numTilesX]);

            draw(vtx1,tex1);


#else
            if(prevX > zoomedImageWidth)
                prevX = zoomedImageWidth;

            if(prevY > zoomedImageHeight)
                prevY = zoomedImageHeight;


            glBindTexture(GL_TEXTURE_2D,imageTextures[i+j*numTilesX]); //m_imageTexture);//

            glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2i(0,0); glVertex2i(tileX,prevY);
            glTexCoord2i(0,1); glVertex2i(tileX,tileY);
            glTexCoord2i(1,0); glVertex2i(prevX,prevY);
            glTexCoord2i(1,1); glVertex2i(prevX,tileY);
            glEnd();
#endif
//            renderText(tileX,tileY,0,QString("%1 %2 ").arg(tileX).arg(tileY));
//            renderText(prevX,prevY,0,QString("%1 %2 ").arg(prevX).arg(prevY));

            tileX=prevX;
            prevX+=zoomedTileWidth;

            if(prevX > zoomedImageWidth)
                prevX = zoomedImageWidth;
        }
    }


    if(m_title != NULL && m_title.isEmpty() == false)
    {
#ifndef GL_ES_VERSION_2_0
        renderText(0.0,0.0,0.0,m_title);
#else

//        txtPixmap.
    QImage base_img(512,64,QImage::Format_ARGB32);
    QPainter p(&base_img);

    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(base_img.rect(), Qt::transparent);
    p.setPen(Qt::blue);
    p.drawText(0,60,m_title);
    p.end();

    GLint textTexture = bindTexture(base_img);
    // Set nearest filtering mode for texture minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Set bilinear filtering mode for texture magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    tileX = 0;
    tileY = 0;
    prevX = 512;
    prevY = -64;
    GLshort vtx1[] = {
        tileX, prevY, 0,
        tileX,tileY, 0,
        prevX, prevY, 0,
        prevX, tileY, 0
    };

    glBindTexture(GL_TEXTURE_2D,textTexture);
    draw(vtx1,tex1);

    deleteTexture(textTexture);
#endif
    }

}


void ImageViewerGL::resizeGL(int w, int h)
{
    drawImage(w,h);
}


void ImageViewerGL::zoomIn()
{
    float oldZoom = zoom;
    zoom*=1.3;
    if(zoom >2.0)
    zoom = 2.0;
    if(oldZoom <1.0 && zoom >1.0)
    zoom = 1.0;
    this->repaint();
}


void ImageViewerGL::zoomOut()
{
    float oldZoom = zoom;
    zoom*=.8;
    if(zoom<.01)
    zoom = .01;
    if(oldZoom >1.0 && zoom <1.0)
    zoom = 1.0;
    this->repaint();
}


void ImageViewerGL::Left()
{
    if(zoom < 0)
    return;
    focusX+=10/zoom;
    this->repaint();
}


void ImageViewerGL::Right()
{
    if(zoom < 0)
    return;
    focusX-=10/zoom;
    this->repaint();
}


void    ImageViewerGL::mouseMoveEvent ( QMouseEvent * event )
{
    int dx = event->x() - lastClick.x();
    int dy = event->y() - lastClick.y();
    lastClick = event->pos();
    focusX -= dx/zoom;
    focusY += dy/zoom;
    this->repaint();
    event->accept();
}


void   ImageViewerGL:: mousePressEvent ( QMouseEvent * event )
{
    lastClick = event->pos();
    event->accept();
}


void     ImageViewerGL::wheelEvent ( QWheelEvent * event )
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    float oldZoom = zoom;
    if(numSteps < 0)
    zoom*=numSteps*-0.8;
    else
    zoom*=numSteps*1.2;
    if(zoom >2.0)
    zoom = 2.0;
    if(zoom<.01)
    zoom = .01;
    if(oldZoom >1.0 && zoom <1.0)
    zoom = 1.0;
    else if(oldZoom <1.0 && zoom >1.0)
    zoom = 1.0;
    this->repaint();
    event->accept();
}


void ImageViewerGL::rotateLeft()
{
    int i,j=rotationAngle;
    for(i=j;i<=j+90;i+=5)
    {
        rotationAngle = i;
        this->repaint();
    }
}


void ImageViewerGL::rotateRight()
{
    int i,j=rotationAngle;
    for(i=j;i>=j-90;i-=5)
    {
        rotationAngle = i;
        this->repaint();
    }
}

void ImageViewerGL::LoadShader(QString vshader, QString fshader)
{
#ifdef GL_ES_VERSION_2_0

    if(program != NULL)
        program->release();
    else
        program = new QOpenGLShaderProgram(this);

    // Override system locale until shaders are compiled
    setlocale(LC_NUMERIC, "C");
    // Compile vertex shader
    if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, vshader))
    close();
    // Compile fragment shader
    if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, fshader))
    close();
    // Link shader pipeline
    if (!program->link())
    close();
    // Bind shader pipeline for use
    if (!program->bind())
    close();
    // Restore system locale
    setlocale(LC_ALL, "");
#endif
}
