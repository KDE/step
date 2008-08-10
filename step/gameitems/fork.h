/* This file is part of StepCore library.
   Copyright (C) 2008 Aliona Kuznetsova <aliona.kuz@gmail.com>

   StepCore library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   StepCore library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with StepCore; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/** \file fork.h
 *  \brief Fork class
 */

#ifndef STEPCORE_FORK_H
#define STEPCORE_FORK_H

#include <stepcore/world.h>
#include <stepcore/object.h>
#include <stepcore/constants.h>
#include <stepcore/rigidbody.h>

#include "worldgraphics.h"

namespace StepCore
{
class Fork: public Box
{
    STEPCORE_OBJECT(Box)

public:
    /** Constructs Fork */
    explicit Fork(Vector2d position = Vector2d(0), double angle = 0,
              Vector2d velocity = Vector2d(0), double angularVelocity = 0,
              double mass = 1, double inertia = 1, Vector2d localSize = Vector2d(2,0.4))
    : Box(position, angle, velocity, angularVelocity, mass, inertia, localSize)
    {}

};

} // namespace StepCore

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

class ForkGraphicsItem: public WorldGraphicsItem {
public:
    ForkGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene);

    QPainterPath shape() const;
   // void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void viewScaleChanged();
    void worldDataChanged(bool);

protected:
    QString pixmapCacheKey();
    QPixmap* paintPixmap();

    StepCore::Fork* fork() const {
        return static_cast<StepCore::Fork*>(_item); }

    QPainterPath _painterPath;
    double _rnorm;
    double _rscale;
    double _radius;
};

class ForkCreator: public ItemCreator
{
public:
    ForkCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);
    void start();

protected:
    StepCore::Vector2d _topLeft;
};


#endif