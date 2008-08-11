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

/** \file baloon.h
 *  \brief Baloon class
 */

#ifndef STEPCORE_BALOON_H
#define STEPCORE_BALOON_H

#include <stepcore/world.h>
#include <stepcore/object.h>
#include <stepcore/constants.h>
#include <stepcore/rigidbody.h>

#include "worldgraphics.h"
namespace StepCore
{
class Baloon: public Disk
{
    STEPCORE_OBJECT(Baloon)

public:
    /** Constructs Baloon */
    explicit Baloon(Vector2d position = Vector2d(0), double angle = 0,
        Vector2d velocity = Vector2d(0), double angularVelocity = 0,
        double mass = 0.01, double inertia = 1, double radius = 0.5)
    : Disk(position, angle, velocity, angularVelocity, mass, inertia, radius)
    {}

    bool checkBreak();

};

} // namespace StepCore

class BaloonGraphicsItem: public WorldGraphicsItem {
public:
    BaloonGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene);

    QPainterPath shape() const;
   // void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void viewScaleChanged();
    void worldDataChanged(bool);

protected:
    QString pixmapCacheKey();
    QPixmap* paintPixmap();

    StepCore::Baloon* baloon() const {
        return static_cast<StepCore::Baloon*>(_item); }

    QPainterPath _painterPath;
    double _rnorm;
    double _rscale;
    double _radius;
};

class BaloonCreator: public ItemCreator
{
public:
    BaloonCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);
    void start();

protected:
    StepCore::Vector2d _topLeft;
};

#endif

