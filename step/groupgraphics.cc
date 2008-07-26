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

#include "groupgraphics.h"
#include <stepcore/world.h>
#include <stepcore/group.h>

#include "worldmodel.h"
#include "worldfactory.h"
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QPainter>
#include <KLocale>
#include <KDebug>

GroupGraphicsItem::GroupGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
    : WorldGraphicsItem(item, worldModel, worldScene)//, _arrows(0)
{
    Q_ASSERT(dynamic_cast<StepCore::Group*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);
    
    _boundingRect = QRectF(0,0,0,0);
    //_boundingRect.moveCenter(QPointF(0,0));
}

QPainterPath GroupGraphicsItem::shape() const
{
    QPainterPath path;
    path.addEllipse(QRectF(-RADIUS-1,-RADIUS-1,(RADIUS+1)*2,(RADIUS+1)*2));
    return path;
}

void GroupGraphicsItem::viewScaleChanged()
{
     worldDataChanged(true);
}

void GroupGraphicsItem::worldDataChanged(bool dynamicOnly)
{
//    setPos(_worldScene->vectorToPoint(particle()->position()));
    _boundingRect = QRectF(0,0,0,0);
    StepCore::ItemList::const_iterator end = group()->items().end();
    for(StepCore::ItemList::const_iterator it = group()->items().begin(); it != end; ++it) {
        WorldGraphicsItem* item = _worldScene->graphicsFromItem(*it);
        if(item){
            item->worldDataChanged(dynamicOnly);
            _boundingRect |= item->sceneTransform().mapRect(item->boundingRect());
        }
    }
}

//void ParticleGraphicsItem::stateChanged()
//{
//    if((_isSelected || _isMouseOverItem) && !_arrows) {
//        _arrows = new ArrowsGraphicsItem(_item, _worldModel, _worldScene, this, "velocity", "acceleration", NULL);
//    }
//    if(!_isMouseOverItem && !_isSelected && _arrows) {
//        delete _arrows; _arrows = 0;
//    }
//}

