

#ifndef WORLDGLSCENE_H
#define WORLDGLSCENE_H
#include <klocalizedstring.h>
#include <QGLWidget>
#include <QMouseEvent>
#include <QMessageBox>

#include <QGLWidget>


#include "worldmodel.h"

#include <QGraphicsView>
#include <QList>
#include <QHash>

#include "messageframe.h"


class KUrl;
class WorldModel;
class QModelIndex;
class QGraphicsItem;
class QItemSelection;
class QVBoxLayout;
class QSignalMapper;
class WorldGraphicsItem;
class WorldGraphicsView;
class ItemCreator;
class WorldSceneAxes;
class WorldFactory;
class QMessageBox;
class QGraphicsItem;

 class Geometry;
 class Cube;
 class Tile;
 class QGLPixelBuffer;

class WorldGLScene : public QGLWidget{
    
    Q_OBJECT
    
public:
      /**Construtor da Classe WorldGLScene*/
     WorldGLScene(QWidget *parent = 0,WorldModel* worldModel=0);
    ~WorldGLScene();
   
protected:
     void initializeGL();
     void paintGL();
     void resizeGL(int width, int height);
     bool event(QEvent* event);
     void createCube(float x);
     void mousePressEvent(QMouseEvent *event);
//      void mouseReleaseEvent(QMouseEvent *) { setAnimationPaused(false); }

private:
     void initializeGeometry();
     void initPbuffer();
     void initCommon();
     void perspectiveProjection();
     void orthographicProjection();
     void drawPbuffer();
     void setAnimationPaused(bool enable);     

public:
     int showMessage(MessageFrame::Type type, const QString& text, MessageFrame::Flags flags = 0) {
        return _messageFrame->showMessage(type, text, flags);
      }
      int changeMessage(int id, MessageFrame::Type type, const QString& text, MessageFrame::Flags flags = 0) {
	  return _messageFrame->changeMessage(id, type, text, flags);
      }
      void closeMessage(int id) { _messageFrame->closeMessage(id); }
      typedef QList<const StepCore::MetaObject*> SnapList;

   
    enum SnapFlag {
        SnapOnCenter = 1,         ///< Snap to the center of the body
        SnapSetPosition = 2,      ///< Set position property
        SnapSetAngle = 4,         ///< Set angle property
        SnapSetLocalPosition = 8, ///< Set localPosition property
        SnapParticle = 256,         ///< Allow snapping to Particle
        SnapRigidBody = 512        ///< Allow snapping to RigidBody
    };
    Q_DECLARE_FLAGS(SnapFlags, SnapFlag)
    
signals:
    /** This signal is emitted when item creation is finished or canceled */
    void endAddItem(const QString& name, bool success);   
    /** This signal is emitted when a link in the message is activated */
    void linkActivated(const KUrl& url);
   
	   
public slots:
	  void messageLinkActivated(const QString& link);



private:
     
     int xRot;
     int yRot;
     int zRot;
     float pos;
     QColor qtGreen;
     QColor qtPurple;
     QMessageBox *msgm;
     MessageFrame*  _messageFrame;
     QGraphicsItem *itemAt(const QPointF &pos, const QTransform &deviceTransform) const;
     Cube* cubo;
     Geometry *geom;
     
     qreal aspect;
     GLuint dynamicTexture;
     GLuint cubeTexture;
     bool hasDynamicTextureUpdate;
     QGLPixelBuffer *pbuffer;
     
   
     Cube *cube;
     Tile *backdrop;
     QList<Cube *> cubes;
     QList<Tile *> tiles;
public:     
     ItemCreator *_itemCreator;
     WorldModel* _worldModel;  
	  


    

};


#endif // WORLDGLSCENE_H