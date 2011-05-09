#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#include <QtOpenGL>
#include <QtGui>
#include <math.h>
#include "arrow.h"
#include "worldglscene.h"
#include "worldgraphics.h"
#include "worldfactory.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <kurl.h>
#include "qgraphicsitem.h"
#include <QPointF>

 #include "cube.h"

 #include <QGLPixelBuffer>

 static GLfloat colorArray[][4] = {
     {0.243f, 0.423f, 0.125f, 1.0f},
     {0.176f, 0.31f, 0.09f, 1.0f},
     {0.4f, 0.69f, 0.212f, 1.0f},
     {0.317f, 0.553f, 0.161f, 1.0f}
 };

WorldGLScene::WorldGLScene(QWidget* parent, WorldModel* worldModel): 
QGLWidget(QGLFormat(QGL::SampleBuffers), parent),_itemCreator(0),geom(0), cubo(0)
{
 
     QGLFormat pbufferFormat = format();
     pbufferFormat.setSampleBuffers(false);
     pbuffer = new QGLPixelBuffer(QSize(512, 512), pbufferFormat, this);
     initializeGeometry();
  
  
     xRot = 0;
     yRot = 0;
     zRot = 0;
     pos = 0.0;
     qtGreen = QColor::fromCmykF(0.40, 0.0, 1.0, 0.0);
     qtPurple = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);
     _worldModel = worldModel;
     _messageFrame = new MessageFrame();
     msgm=new QMessageBox();
  
     connect(_messageFrame, SIGNAL(linkActivated(const QString&)),
                this, SLOT(messageLinkActivated(const QString&)));
   
    
}

WorldGLScene::~WorldGLScene(){
     pbuffer->releaseFromDynamicTexture();
     glDeleteTextures(1, &dynamicTexture);
     delete pbuffer;

     qDeleteAll(cubes);
     qDeleteAll(tiles);
     delete cubo;
//     delete cube;
}


void WorldGLScene::initializeGL()
 {
      initCommon();
      glShadeModel(GL_SMOOTH);
      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);
      static GLfloat lightPosition[4] = { 0.5, 5.0, 7.0, 1.0 };
      glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
      initPbuffer();
    
 }
 
void WorldGLScene::createCube(float x)
{
  geom = new Geometry();
  CubeBuilder cBuilder(geom, 0.5);
  cBuilder.setColor(qtPurple.dark());
  
  cubes.append(cBuilder.newCube(QVector3D(pos, pos, pos)));
  pos=pos+0.2;
  this->updateGL();
 
}
 
 
void WorldGLScene::paintGL(){
      QPainter* painter = new QPainter;
      pbuffer->makeCurrent();
      drawPbuffer();

     makeCurrent();

  
     // set up to render the scene
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     glLoadIdentity();
     glTranslatef(0.0f, 0.0f, -10.0f);
     QPen pen(Qt::gray, 2);
     pen.setCosmetic(true);
     painter->setPen(pen);//, Qt::DotLine, Qt::SquareCap, Qt::RoundJoin));
   
    Arrow arrow(0, 100, 0, -100, 8);
    arrow.draw(painter);
    Arrow arrow2(-100, 0, 100, 0, 8);
    arrow2.draw(painter);
    

    painter->drawText(QRectF(-100 - 6, 6, 100, 100),
                        Qt::AlignRight | Qt::AlignTop,
                        QString("%1,%2").arg( 0.5, 0, 'g', 3 ).arg( 0.5, 0, 'g', 3 ));
    painter->drawText(QRectF(6, -100, 100 - 6, 100 - 6),
            Qt::AlignLeft | Qt::AlignTop, QString::number( 0.0 + 100/10, 'g', 3 ));
    painter->drawText(QRectF(6, -100, 100 - 6, 100 - 6),
            Qt::AlignRight | Qt::AlignBottom, QString::number(0.0 + 100/10, 'g', 3 ));


     // draw the bouncing cubes
      for (int i = 0; i < cubes.count(); ++i){
          cubes[i]->draw();
      }

}

void WorldGLScene::beginAddItem(const QString& name)
{
    //_currentCreator = name;
    if(_itemCreator) {
        _itemCreator->abort();
        emit endAddItem(_itemCreator->className(), _itemCreator->item() != NULL);
        delete _itemCreator;
    }
    if(name == "Pointer") {
        _itemCreator = NULL;
    } else {
        _itemCreator = _worldModel->worldFactory()->newItemCreator3D(name, _worldModel, this);
        Q_ASSERT(_itemCreator != NULL);
        _itemCreator->start();
    }
}

void WorldGLScene::initializeGeometry()
 {
     geom = new Geometry();
     CubeBuilder cBuilder(geom, 0.5);
     cBuilder.setColor(qtPurple.dark());
   
 }

void WorldGLScene::initCommon()
 {
     glClearColor(1.0f ,1.0f , 1.0f ,1.0f);

     glEnable(GL_DEPTH_TEST);
     glEnable(GL_CULL_FACE);
     glEnable(GL_MULTISAMPLE);

     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     glEnable(GL_BLEND);
     geom->loadArrays();
 }
 
 
  void WorldGLScene::perspectiveProjection()
 {
     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
 #ifdef QT_OPENGL_ES
     glFrustumf(-aspect, +aspect, -1.0, +1.0, 4.0, 15.0);
 #else
     glFrustum(-aspect, +aspect, -1.0, +1.0, 4.0, 15.0);
 #endif
     glMatrixMode(GL_MODELVIEW);
 }

 void WorldGLScene::orthographicProjection()
 {
     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
 #ifdef QT_OPENGL_ES
     glOrthof(-1.0, +1.0, -1.0, +1.0, -90.0, +90.0);
 #else
     glOrtho(-1.0, +1.0, -1.0, +1.0, -90.0, +90.0);
 #endif
     glMatrixMode(GL_MODELVIEW);
 }
 


void WorldGLScene::resizeGL(int width, int height)
 {
     glViewport(0, 0, width, height);
     aspect = (qreal)width / (qreal)(height ? height : 1);
     perspectiveProjection();
 }
 

 void WorldGLScene::drawPbuffer()
 {
     orthographicProjection();

     glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     glEnable(GL_CULL_FACE);
     glFlush();
 }

 void WorldGLScene::initPbuffer()
 {
    pbuffer->makeCurrent();
    initCommon();
    makeCurrent();
 }

void WorldGLScene::mousePressEvent(QMouseEvent* event)
{
      QPointF ponto= event->posF(); 
      createCube(0.0);
      cubes[cubes.length()-1]->startAnimation();
      connect(cubes[cubes.length()-1],SIGNAL(changed()),this,SLOT(updateGL()));
      
      
      QWidget::mousePressEvent(event);
 }

void WorldGLScene::messageLinkActivated(const QString& link)
{
    emit linkActivated(link);
}

bool WorldGLScene::event(QEvent* event)
{
   
    if(_itemCreator) {
     
        bool handled = _itemCreator->sceneEvent(event);
        if(_itemCreator->finished()) {
	  
            emit endAddItem(_itemCreator->className(), _itemCreator->item() != NULL);
	
            ItemCreator* itemCreator = _itemCreator;
            _itemCreator = NULL;
            delete itemCreator;
        }
        if(handled) {
            event->accept();
            return true;
        }
    }
    return QGLWidget::event(event);
}

