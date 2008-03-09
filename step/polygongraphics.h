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

namespace StepCore {
    class RigidBody;
    class Disk;
    class Polygon;
}

class RigidBodyGraphicsItem: public WorldGraphicsItem
{
public:
    RigidBodyGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void viewScaleChanged();
    void stateChanged();
    void worldDataChanged(bool dynamicOnly);

protected:
    StepCore::RigidBody* rigidBody() const;
    QPainterPath _painterPath;

    ArrowHandlerGraphicsItem*         _velocityHandler;
    CircularArrowHandlerGraphicsItem* _angularVelocityHandler;
    CircularArrowHandlerGraphicsItem* _angleHandler;
};

class DiskCreator: public ItemCreator
{
public:
    DiskCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);
    void start();
};

class DiskGraphicsItem: public RigidBodyGraphicsItem
{
public:
    DiskGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    void viewScaleChanged();

protected:
    StepCore::Disk* disk() const;
};

class PolygonCreator: public ItemCreator
{
public:
    PolygonCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);
    void start();

protected:
    void fixCenterOfMass();
    void fixInertia();
};

class PolygonGraphicsItem: public RigidBodyGraphicsItem
{
public:
    PolygonGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    void viewScaleChanged();

protected:
    StepCore::Polygon* polygon() const;
};

#endif

