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

#include "gasgraphics.h"

#include "worldmodel.h"
#include "worldfactory.h"
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QEvent>
#include <QPainter>
#include <KLocale>

double GasCreator::random11()
{
    return double(qrand()) / (RAND_MAX/2) - 1;
}

bool GasCreator::sceneEvent(QEvent* event)
{
    if(event->type() == QEvent::GraphicsSceneMousePress) {
        _worldModel->simulationPause();

        _worldModel->beginMacro(i18n("Create %1", _className));
        _item = _worldModel->newItem(_className); Q_ASSERT(_item != NULL);
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);

        StepCore::Gas* gas = static_cast<StepCore::Gas*>(_item);
        _worldModel->newItem("GasLJForce", gas);
        StepCore::Object* ljforce = gas->items()[0];
        _worldModel->setProperty(ljforce, ljforce->metaObject()->property("depth"), 0.1);
        _worldModel->setProperty(ljforce, ljforce->metaObject()->property("rmin"), 0.1);

        for(int i=0; i<20; ++i) {
            _worldModel->newItem("GasParticle", gas);
        }
        for(int i=0; i<20; ++i) {
            StepCore::Object* obj = gas->items()[1+i];
            StepCore::Vector2d pos(random11(), random11());
            StepCore::Vector2d vel(random11()/4, random11()/4);
            _worldModel->setProperty( obj, obj->metaObject()->property("position"), QVariant::fromValue(pos) );
            _worldModel->setProperty( obj, obj->metaObject()->property("velocity"), QVariant::fromValue(vel) );
        }

        _worldModel->endMacro();
        event->accept();
        return true;
    }
    return false;

}

class GasArrowHandlerGraphicsItem: public ArrowHandlerGraphicsItem
{
public:
    GasArrowHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                        QGraphicsItem* parent, const StepCore::MetaProperty* property)
        : ArrowHandlerGraphicsItem(item, worldModel, parent, property) {}

protected:
    StepCore::Vector2d value() {
        return static_cast<StepCore::Gas*>(_item)->measureRectCenter() +
                static_cast<StepCore::Gas*>(_item)->measureRectSize()/2.0;
    }
    void setValue(const StepCore::Vector2d& value) {
        _worldModel->simulationPause();
        StepCore::Vector2d v = (value - static_cast<StepCore::Gas*>(_item)->measureRectCenter())*2.0;
        _worldModel->setProperty(_item, _property, QVariant::fromValue(v));
    }
};

GasGraphicsItem::GasGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Gas*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    //setFlag(QGraphicsItem::ItemIsMovable);
    //setAcceptsHoverEvents(true);
    _centerHandler = new ArrowHandlerGraphicsItem(item, worldModel, this,
                            _item->metaObject()->property("measureRectCenter"));
    _sizeHandler = new GasArrowHandlerGraphicsItem(item, worldModel, this,
                            _item->metaObject()->property("measureRectSize"));
    _centerHandler->setVisible(false);
    _sizeHandler->setVisible(false);
    setZValue(HANDLER_ZVALUE);
}

inline StepCore::Gas* GasGraphicsItem::gas() const
{
    return static_cast<StepCore::Gas*>(_item);
}

QPainterPath GasGraphicsItem::shape() const
{
    QPainterPath path;
    //path.addRect(QRectF(-radius,-radius,radius*2,radius*2));
    return path;
}

void GasGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    if(_isSelected) {
        painter->setPen(QPen(Qt::red, 0));
        painter->drawRect(_boundingRect);
    }
}

void GasGraphicsItem::viewScaleChanged()
{
}

void GasGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) stateChanged();
}

void GasGraphicsItem::stateChanged()
{
    prepareGeometryChange();
    if(_isSelected) {
        const StepCore::Vector2d& size = gas()->measureRectSize();
        StepCore::Vector2d r0 = gas()->measureRectCenter() - size/2.0;
        _boundingRect = QRectF(r0[0], r0[1], size[0], size[1]);
        _centerHandler->setVisible(true);
        _sizeHandler->setVisible(true);
    }
    else {
        _boundingRect = QRectF();
        _centerHandler->setVisible(false);
        _sizeHandler->setVisible(false);
    }
    update();
}

