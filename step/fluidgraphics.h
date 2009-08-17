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

#ifndef STEP_FLUIDGRAPHICS_H
#define STEP_FLUIDGRAPHICS_H

#include "worldgraphics.h"

namespace StepCore {
    class Fluid;
}

class FluidCreator: public ItemCreator
{
public:
    FluidCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
           : ItemCreator(className, worldModel, worldScene) {}

    bool sceneEvent(QEvent* event);
    void start();

protected:
    StepCore::Vector2d _topLeft;
};

class FluidVertexHandlerGraphicsItem: public OnHoverHandlerGraphicsItem
{
    Q_OBJECT

public:
    FluidVertexHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                        QGraphicsItem* parent, int vertexNum)
        : OnHoverHandlerGraphicsItem(item, worldModel, parent, NULL, NULL, vertexNum) {}

protected:
    StepCore::Fluid* fluid() const;
    StepCore::Vector2d value();
    void setValue(const StepCore::Vector2d& value);
};

class FluidGraphicsItem: public WorldGraphicsItem {
public:
    FluidGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void viewScaleChanged();
    void stateChanged();
    void worldDataChanged(bool);

protected: 
    void mouseSetPos(const QPointF& pos, const QPointF&, MovingState);
    OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF& pos);
    StepCore::Fluid* fluid() const;
    QRectF _measureRect;

    //ArrowHandlerGraphicsItem *_centerHandler;
};

namespace Ui {
    class WidgetCreateFluidParticles;
}

class FluidKDialog;
class FluidMenuHandler: public ItemMenuHandler
{
    Q_OBJECT

public:
    FluidMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
        : ItemMenuHandler(object, worldModel, parent) {}

    void populateMenu(QMenu* menu);

public slots:
    void createFluidParticles();

protected slots:
    bool createFluidParticlesApply();
    void createFluidParticlesCountChanged();
    void createFluidParticlesConcentrationChanged();
    void clearFluid();

protected:
    StepCore::Fluid* fluid() const;
    Ui::WidgetCreateFluidParticles* _createFluidParticlesUi;
    FluidKDialog*                   _createFluidParticlesDialog;
    friend class FluidKDialog;
    static const int MAX_PARTICLES = 200;
//    bool                      _confChanged;
};

#endif

