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

#ifndef STEP_POLYGONGRAPHICS_H
#define STEP_POLYGONGRAPHICS_H

/*+++++++++ XXX +++++++++++
 * This need to be redone
 */

#include "worldgraphics.h"
#include <QPainterPath>
#include <stepcore/rigidbody.h>

class PolygonCreator: public ItemCreator
{
public:
    PolygonCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);

protected:
    void fixCenterOfMass();
    void fixInertia();
};

class PolygonGraphicsItem: public WorldGraphicsItem {
public:
    PolygonGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    ~PolygonGraphicsItem();

    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void advance(int phase);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    void mouseSetPos(const QPointF& pos, const QPointF& diff);
    StepCore::Polygon* polygon() const;
    QPainterPath _painterPath;

    ArrowHandlerGraphicsItem *_velocityHandler;

    static const int RADIUS = 7;
};

#endif

