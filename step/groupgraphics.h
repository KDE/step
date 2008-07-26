/* This file is part of Step.
   Copyright (C) 2008 Aliona Kuznetsova <aliona.kuz@gmail.com>

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

#ifndef STEP_GROUPGRAPHICS_H
#define STEP_GROUPGRAPHICS_H

#include "worldgraphics.h"
#include <stepcore/world.h>
#include <stepcore/group.h>

class GroupGraphicsItem: public WorldGraphicsItem {
public:
    GroupGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene);

    QPainterPath shape() const;
    //void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    
    void viewScaleChanged();
//    void stateChanged();
    void worldDataChanged(bool);

protected:
    StepCore::Group* group() const {
        return static_cast<StepCore::Group*>(_item); }

    //LinearArrowHandlerGraphicsItem *_velocityHandler;
    //ArrowsGraphicsItem* _arrows;

    static const int RADIUS = 7;
};

#endif

