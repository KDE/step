/* This file is part of Step.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

   Step is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Step is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Step; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "fluidgraphics.h"
#include "fluidgraphics.moc"

#include <stepcore/fluid.h>

#include "ui_create_fluid_particles.h"

#include "worldmodel.h"
#include "worldfactory.h"
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QEvent>
#include <QPainter>
#include <QValidator>
#include <QApplication>
#include <KDialog>
#include <KMessageBox>
#include <KLocale>
#include <KDebug>

#include <float.h>

void FluidCreator::start()
{
    showMessage(MessageFrame::Information,
            i18n("Press left mouse button to position\ntop left corner of a region for %1", classNameTr()));
}

bool FluidCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

    if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));

        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->newItem(_className); Q_ASSERT(_item != NULL);
        _worldModel->setProperty(_item, "measureRectCenter", vpos);
        _worldModel->setProperty(_item, "measureRectSize", QVariant::fromValue(StepCore::Vector2d::Zero().eval()));
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);

        StepCore::Fluid* fluid = static_cast<StepCore::Fluid*>(_item);
        _worldModel->newItem("FluidForce", fluid);
        //StepCore::Object* fluidforce = fluid->items()[0];
        //_worldModel->setProperty(fluidforce, "skradius", 2.0);

        _topLeft = WorldGraphicsItem::pointToVector(pos);

        showMessage(MessageFrame::Information,
            i18n("Move mouse and release left mouse button to position\nbottom right corner of the region for %1", classNameTr()));

        return true;
    } else if(event->type() == QEvent::GraphicsSceneMouseMove &&
                    mouseEvent->buttons() & Qt::LeftButton) {
        
        _worldModel->simulationPause();
        StepCore::Vector2d pos = WorldGraphicsItem::pointToVector(mouseEvent->scenePos());
        StepCore::Vector2d position = (_topLeft + pos) / 2.0;
        StepCore::Vector2d size = _topLeft - pos;
        _worldModel->setProperty(_item, "measureRectCenter", QVariant::fromValue(position));
        _worldModel->setProperty(_item, "measureRectSize", QVariant::fromValue(size));
        return true;

    } else if(event->type() == QEvent::GraphicsSceneMouseRelease &&
                    mouseEvent->button() == Qt::LeftButton) {

        _worldModel->simulationPause();
        StepCore::Vector2d pos = WorldGraphicsItem::pointToVector(mouseEvent->scenePos());
        StepCore::Vector2d position = (_topLeft + pos) / 2.0;
        StepCore::Vector2d size = _topLeft - pos;
        if(size[0] == 0 && size[1] == 0) { size[0] = size[1] = 1; }
        _worldModel->setProperty(_item, "measureRectCenter", QVariant::fromValue(position));
        _worldModel->setProperty(_item, "measureRectSize", QVariant::fromValue(size));

        showMessage(MessageFrame::Information,
            i18n("Please fill in the parameters for the fluid particles."));

        FluidMenuHandler* menuHandler = new FluidMenuHandler(_item, _worldModel, NULL);
        menuHandler->createFluidParticles();
        menuHandler->deleteLater();

        _worldModel->endMacro();

        showMessage(MessageFrame::Information,
            i18n("%1 named '%2' created", classNameTr(), _item->name()),
            MessageFrame::CloseButton | MessageFrame::CloseTimer);

        setFinished();
        return true;
    }

    return false;
}

/*
class FluidArrowHandlerGraphicsItem: public ArrowHandlerGraphicsItem
{
public:
    FluidArrowHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                        QGraphicsItem* parent, const StepCore::MetaProperty* property)
        : ArrowHandlerGraphicsItem(item, worldModel, parent, property) {}

protected:
    StepCore::Vector2d value() {
        return static_cast<StepCore::Fluid*>(_item)->measureRectCenter() +
                static_cast<StepCore::Fluid*>(_item)->measureRectSize()/2.0;
    }
    void setValue(const StepCore::Vector2d& value) {
        _worldModel->simulationPause();
        StepCore::Vector2d v = (value - static_cast<StepCore::Fluid*>(_item)->measureRectCenter())*2.0;
        _worldModel->setProperty(_item, _property, QVariant::fromValue(v));
    }
};
*/

inline StepCore::Fluid* FluidVertexHandlerGraphicsItem::fluid() const
{
    return static_cast<StepCore::Fluid*>(_item);
}

StepCore::Vector2d FluidVertexHandlerGraphicsItem::value() {
    return fluid()->measureRectSize().cwise()*(corners[_vertexNum]);
}

void FluidVertexHandlerGraphicsItem::setValue(const StepCore::Vector2d& value)
{
    StepCore::Vector2d oCorner = fluid()->measureRectCenter() -
                    fluid()->measureRectSize().cwise()*(corners[_vertexNum]);

    StepCore::Vector2d delta = (fluid()->measureRectCenter() + value - oCorner)/2.0;
    StepCore::Vector2d newPos = oCorner + delta;
    StepCore::Vector2d newSize = (newPos - oCorner)*2.0;

    double d = -0.1/currentViewScale();
    StepCore::Vector2d sign = delta.cwise()*(corners[_vertexNum]);
    if(sign[0] < d || sign[1] < d) {
        if(sign[0] < d) {
            newPos[0] = oCorner[0]; newSize[0] = 0;
            _vertexNum ^= 1;
        }
        if(sign[1] < d) {
            newPos[1] = oCorner[1]; newSize[1] = 0;
            _vertexNum ^= 2;
        }
        _worldModel->setProperty(_item, "measureRectCenter", QVariant::fromValue(newPos));
        _worldModel->setProperty(_item, "measureRectSize", QVariant::fromValue(newSize));
        setValue(value);
        return;
    }

    _worldModel->setProperty(_item, "measureRectCenter", QVariant::fromValue(newPos));
    _worldModel->setProperty(_item, "measureRectSize", QVariant::fromValue(newSize));
}

FluidGraphicsItem::FluidGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Fluid*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    //setExclusiveMoving(true);
    //setFlag(QGraphicsItem::ItemIsMovable);
    //setAcceptsHoverEvents(true);
    //_centerHandler = new ArrowHandlerGraphicsItem(item, worldModel, this,
    //                        _item->metaObject()->property("measureRectCenter"));
    //_centerHandler->setVisible(false);
    setZValue(REGION_ZVALUE);
    setAcceptsHoverEvents(true);
    setOnHoverHandlerEnabled(true);
}

inline StepCore::Fluid* FluidGraphicsItem::fluid() const
{
    return static_cast<StepCore::Fluid*>(_item);
}

void FluidGraphicsItem::mouseSetPos(const QPointF& pos, const QPointF&, MovingState)
{
#ifdef __GNUC__
#warning Consider renaming measureRectCenter to position
#endif
    const StepCore::MetaProperty* property = _item->metaObject()->property("measureRectCenter");
    _worldModel->simulationPause();
    _worldModel->setProperty(_item, property, QVariant::fromValue( pointToVector(pos) ));
}

QPainterPath FluidGraphicsItem::shape() const
{
    QPainterPath path;
    //path.addRect(QRectF(-radius,-radius,radius*2,radius*2));
    path.setFillRule(Qt::WindingFill);
    //path.addRect(_measureRect);
    path.addRect(_boundingRect);

    return path;
}

void FluidGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    //if(_isSelected) {
        const StepCore::Vector2d& size = fluid()->measureRectSize();
        painter->setPen(QPen(QColor::fromRgba(fluid()->color()), 0));
        painter->drawRect(QRectF(-size[0]/2, -size[1]/2, size[0], size[1]));
    //}
        painter->setPen(QPen(Qt::blue, 0.2, Qt::SolidLine, Qt::RoundCap));
        double precision = (fluid()->skradius())*25;

        StepCore::Vector2d r0 = StepCore::Vector2d(-size[0]/2,-size[1]/2);
        StepCore::Vector2d center = fluid()->measureRectCenter();
        StepCore::Vector2d delta = StepCore::Vector2d(size[0]/precision,size[1]/precision);
        StepCore::Vector2d rpoint;
        //qDebug("CENTER IS %f %f - DELTA %f %f",center[0],center[1], delta[0],delta[1]);

        //determine the density range
        double mindensity = 100000;
        double maxdensity = 0;
	for(int i=0; i < precision-1; ++i) {
    	   for(int j=0; j < precision-1; ++j) {
              rpoint = StepCore::Vector2d(r0[0]+0.5*delta[0]+i*delta[0],r0[1]+0.5*delta[1]+j*delta[1]);
              double density = fluid()->calcDensity(rpoint);
              if (density < mindensity){ mindensity = density; }
              if (density > maxdensity){ maxdensity = density; }
           }
        }
        double avgdensity = (maxdensity-mindensity)/2.0;

        for(int i=0; i < precision-1; ++i) {
    	   for(int j=0; j < precision-1; ++j) {
              rpoint = StepCore::Vector2d(r0[0]+0.5*delta[0]+i*delta[0],r0[1]+0.5*delta[1]+j*delta[1]);
              double density = fluid()->calcDensity(rpoint);
              //qDebug("%f %f",density,precision);
              if (density > avgdensity) {
                 //qDebug("%d %d %f - r point - %f %f",i,j,fluid()->calcDensity(rpoint), rpoint[0],rpoint[1]);
                 painter->setOpacity(density/(maxdensity-mindensity));
	         //painter->drawEllipse(QRectF(rpoint[0],rpoint[1],
                 //                            radius,radius));
		 
                 painter->drawPoint(QGraphicsItem::mapFromScene(QPointF(rpoint[0],rpoint[1])));
              }
           }
        }
}

void FluidGraphicsItem::viewScaleChanged()
{
    double s = currentViewScale();
    prepareGeometryChange();
    //const StepCore::Vector2d& size = fluid()->measureFluidSize();
    const StepCore::Vector2d& size = fluid()->measureRectSize();

    /*//Must double check the /s factor is necessary!
    if (size[0] > SCENE_LENGTH / s) {
       size[0]=SCENE_LENGTH / s;
    }
    if (size[1] > SCENE_LENGTH / s) {
       size[1]=SCENE_LENGTH / s;
    }*/

    //position the boundingRect about the origin
    _boundingRect = QRectF(-(size[0]+SELECTION_MARGIN/s)/2, -(size[1]+SELECTION_MARGIN/s)/2,
                            (size[0]+SELECTION_MARGIN/s),    (size[1]+SELECTION_MARGIN/s));

    //_measureRect = QRectF(-(rectsize[0]+SELECTION_MARGIN/s)/2, -(rectsize[1]+SELECTION_MARGIN/s)/2,
    //                        (rectsize[0]+SELECTION_MARGIN/s),    (rectsize[1]+SELECTION_MARGIN/s));
    update();
}

void FluidGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) {
        setPos(vectorToPoint(fluid()->measureRectCenter()));
        viewScaleChanged();
        /*prepareGeometryChange();
        const StepCore::Vector2d& size = fluid()->measureRectSize();
        StepCore::Vector2d r0 = fluid()->measureRectCenter() - size/2.0;
        _boundingRect = QRectF(r0[0], r0[1], size[0], size[1]);*/
        //stateChanged();
    }
}

void FluidGraphicsItem::stateChanged()
{
#if 0
    if(_isSelected) {
        /*
        const StepCore::Vector2d& size = fluid()->measureRectSize();
        StepCore::Vector2d r0 = fluid()->measureRectCenter() - size/2.0;
        _boundingRect = QRectF(r0[0], r0[1], size[0], size[1]);
        */
        _centerHandler->setVisible(true);
    }
    else {
        //_boundingRect = QRectF();
        _centerHandler->setVisible(false);
    }
#endif
    update();
}

OnHoverHandlerGraphicsItem* FluidGraphicsItem::createOnHoverHandler(const QPointF& pos)
{
    double s = currentViewScale();
    StepCore::Vector2d size = fluid()->measureRectSize();
    StepCore::Vector2d l = pointToVector(pos) - fluid()->measureRectCenter();

    int num = -1; double minDist2 = HANDLER_SNAP_SIZE*HANDLER_SNAP_SIZE/s/s;
    for(unsigned int i=0; i<4; ++i) {
        double dist2 = (l - size.cwise()*(OnHoverHandlerGraphicsItem::corners[i])).squaredNorm();
        if(dist2 < minDist2) { num = i; minDist2 = dist2; }
    }

    if(_onHoverHandler && _onHoverHandler->vertexNum() == num)
        return _onHoverHandler;

    if(num >= 0)
        return new FluidVertexHandlerGraphicsItem(_item, _worldModel, this, num);

    return 0;
}

class FluidKDialog: public KDialog
{
public:
    FluidKDialog(FluidMenuHandler* handler, QWidget *parent=0, Qt::WFlags flags=0)
        : KDialog(parent, flags), _handler(handler) {}
protected slots:
    void slotButtonClicked(int button) {
        if(button == KDialog::Ok) {
            if(_handler->createFluidParticlesApply()) accept();
        } else {
            KDialog::slotButtonClicked(button);
        }
    }
    FluidMenuHandler* _handler;
};

void FluidMenuHandler::populateMenu(QMenu* menu)
{
    _createFluidParticlesUi = 0;
    _createFluidParticlesDialog = 0;
    //_confChanged = false;

    menu->addAction(KIcon("step_object_FluidParticle"), i18n("Create particles..."), this, SLOT(createFluidParticles()));
    //menu->addAction(KIcon("edit-clear"), i18n("Clear fluids"), this, SLOT(clearFluid()));
    menu->addSeparator();
    ItemMenuHandler::populateMenu(menu);
}

inline StepCore::Fluid* FluidMenuHandler::fluid() const
{
    return static_cast<StepCore::Fluid*>(_object);
}

void FluidMenuHandler::clearFluid()
{
//    _worldModel->simulationPause();

}

void FluidMenuHandler::createFluidParticles()
{
    if(_worldModel->isSimulationActive())
        _worldModel->simulationStop();

    _createFluidParticlesDialog = new FluidKDialog(this); // XXX: parent?
    
    _createFluidParticlesDialog->setCaption(i18n("Create fluid particles"));
    _createFluidParticlesDialog->setButtons(KDialog::Ok | KDialog::Cancel);

    _createFluidParticlesUi = new Ui::WidgetCreateFluidParticles;
    _createFluidParticlesUi->setupUi(_createFluidParticlesDialog->mainWidget());

    _createFluidParticlesUi->lineEditMass->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createFluidParticlesUi->lineEditMass));
    _createFluidParticlesUi->lineEditCount->setValidator(
                new QIntValidator(0, INT_MAX, _createFluidParticlesUi->lineEditCount));
    _createFluidParticlesUi->lineEditConcentration->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createFluidParticlesUi->lineEditConcentration));
    _createFluidParticlesUi->lineEditTemperature->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createFluidParticlesUi->lineEditTemperature));
    _createFluidParticlesUi->lineEditMeanVelocity->setValidator(
                new QRegExpValidator(QRegExp("^\\([+-]?\\d+(\\.\\d*)?([eE]\\d*)?,[+-]?\\d+(\\.\\d*)?([eE]\\d*)?\\)$"),
                        _createFluidParticlesUi->lineEditMeanVelocity));
    _createFluidParticlesUi->lineEditTemperature->setValidator(
                new QDoubleValidator(0, 3, DBL_DIG, _createFluidParticlesUi->smoothingSlider));

    _createFluidParticlesUi->lineEditVolume->setText(QString::number(fluid()->rectVolume()));
    createFluidParticlesCountChanged();

    _createFluidParticlesUi->labelVolume->setText(fluid()->metaObject()->property("rectVolume")->units());
    _createFluidParticlesUi->labelCount->setText(fluid()->metaObject()->property("rectParticleCount")->units());
    _createFluidParticlesUi->labelConcentration->setText(fluid()->metaObject()->property("rectConcentration")->units());
    _createFluidParticlesUi->labelMass->setText(fluid()->metaObject()->property("rectMeanParticleMass")->units());
    _createFluidParticlesUi->labelTemperature->setText(fluid()->metaObject()->property("rectTemperature")->units());
    _createFluidParticlesUi->labelMeanVelocity->setText(fluid()->metaObject()->property("rectMeanVelocity")->units());

    connect(_createFluidParticlesUi->lineEditCount, SIGNAL(textEdited(const QString&)),
                this, SLOT(createFluidParticlesCountChanged()));
    connect(_createFluidParticlesUi->lineEditConcentration, SIGNAL(textEdited(const QString&)),
                this, SLOT(createFluidParticlesConcentrationChanged()));

    connect(_createFluidParticlesDialog, SIGNAL(okClicked()), this, SLOT(createFluidParticlesApply()));

    _createFluidParticlesDialog->exec();

    delete _createFluidParticlesDialog; _createFluidParticlesDialog = 0;
    delete _createFluidParticlesUi; _createFluidParticlesUi = 0;
}

void FluidMenuHandler::createFluidParticlesCountChanged()
{
    _createFluidParticlesUi->lineEditConcentration->setText(QString::number(
                    _createFluidParticlesUi->lineEditCount->text().toDouble() / fluid()->rectVolume()
                ));
}

void FluidMenuHandler::createFluidParticlesConcentrationChanged()
{
    _createFluidParticlesUi->lineEditCount->setText(QString::number(
                    round(_createFluidParticlesUi->lineEditConcentration->text().toDouble() * fluid()->rectVolume())
                ));
}

bool FluidMenuHandler::createFluidParticlesApply()
{
    Q_ASSERT(_createFluidParticlesUi && _createFluidParticlesDialog);

    int count = _createFluidParticlesUi->lineEditCount->text().toInt();

    if(count > MAX_PARTICLES) {
        int ret = KMessageBox::warningContinueCancel(NULL, 
              i18n("You are trying to create a very large number of particles. "
                   "This will make simulation very slow. Do you want to continue?"),
              i18n("Warning - Step"));
        if(ret != KMessageBox::Continue) return false;
    }

    double mass = _createFluidParticlesUi->lineEditMass->text().toDouble();
    double temperature = _createFluidParticlesUi->lineEditTemperature->text().toDouble();
    double smoothing = double(_createFluidParticlesUi->smoothingSlider->value())/10.0;

    bool ok;
    StepCore::Vector2d meanVelocity = StepCore::stringToType<StepCore::Vector2d>(
                    _createFluidParticlesUi->lineEditMeanVelocity->text(), &ok);

    //StepCore::Fluid* fluid = static_cast<StepCore::Fluid*>(_item);
    //_worldModel->newItem("FluidForce", fluid);
    //StepCore::Object* fluidforce = fluid->items()[0];

    StepCore::Object* fluidforce = fluid()->items()[0];
    _worldModel->setProperty(fluidforce, "skradius", smoothing);
    _worldModel->beginMacro(i18n("Create particles for %1", fluid()->name()));

    std::vector<StepCore::FluidParticle*> particles =
            fluid()->rectCreateParticles(count, mass, temperature, meanVelocity);


    const StepCore::FluidParticleList::const_iterator end = particles.end();
    for(StepCore::FluidParticleList::const_iterator it = particles.begin(); it != end; ++it) {
	//qDebug("INITIAL? pressure=(%f) density=(%f)", it->pressure(), it->density());
        _worldModel->addItem(*it, fluid());
    }

    _worldModel->endMacro();

    return true;
}

/*
void FluidMenuHandler::clearFluid()
{
    _worldModel->simulationPause();
    //_lastPointTime = -HUGE_VAL; // XXX
    _worldModel->setProperty(fluid(), property("points"), QVariant::fromValue(StepCore::Vector2dList()) );
}
*/


