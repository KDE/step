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

#ifndef STEP_SOFTBODYGRAPHICS_H
#define STEP_SOFTBODYGRAPHICS_H

#include "worldgraphics.h"
#include "particlegraphics.h"
#include "springgraphics.h"

namespace Ui {
    class WidgetCreateSoftBodyItems;
}

namespace StepCore{
    class SoftBody;
}

class SoftBodyCreator: public ItemCreator
{
public:
    SoftBodyCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
           : ItemCreator(className, worldModel, worldScene) {}

    bool sceneEvent(QEvent* event);
    void start();
    
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class KDialog;
class SoftBodyMenuHandler: public ItemMenuHandler
{
    Q_OBJECT

public:
    SoftBodyMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
        : ItemMenuHandler(object, worldModel, parent), _applied(false) {}

    void populateMenu(QMenu* menu, KActionCollection* actions);

    bool applied() const { return _applied; }

public slots:
    void createSoftBodyItems(const StepCore::Vector2d& pos);

protected slots:
    void createSoftBodyItemsApply();
    void clearSoftBody();

protected:
    StepCore::SoftBody* softBody() const;
    Ui::WidgetCreateSoftBodyItems* _createSoftBodyItemsUi;
    KDialog*                       _createSoftBodyItemsDialog;
    bool                           _applied;
//    bool                      _confChanged;
};

class SoftBodyGraphicsItem: public WorldGraphicsItem
{
public:
    SoftBodyGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void viewScaleChanged();
    void stateChanged();
    void worldDataChanged(bool dynamicOnly);

protected:
    void mouseSetPos(const QPointF& pos, const QPointF& diff, MovingState);
    StepCore::SoftBody* softBody() const;
    QPainterPath _painterPath;

    ArrowHandlerGraphicsItem *_velocityHandler;

    static const int RADIUS = 7;
};

class SoftBodyParticleGraphicsItem: public ParticleGraphicsItem
{
public:
    SoftBodyParticleGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
        : ParticleGraphicsItem(item, worldModel) {}

    void worldDataChanged(bool dynamicOnly);
};

class SoftBodySpringGraphicsItem: public SpringGraphicsItem
{
public:
    SoftBodySpringGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
        : SpringGraphicsItem(item, worldModel) {}

    void worldDataChanged(bool dynamicOnly);
};

#endif

