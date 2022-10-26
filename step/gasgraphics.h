/* This file is part of Step.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
   Copyright (C) 2014 Inge Wallin        <inge@lysator.liu.se>

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

#ifndef STEP_GASGRAPHICS_H
#define STEP_GASGRAPHICS_H

#include "worldgraphics.h"
#include "stepgraphicsitem.h"
#include "gascreationdialog.h"


namespace StepCore {
    class Gas;
}

class GasCreator: public ItemCreator
{
public:
    GasCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
           : ItemCreator(className, worldModel, worldScene) {}

    bool sceneEvent(QEvent* event) override;
    void start() override;

protected:
    StepCore::Vector2d _topLeft;

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class GasVertexHandlerGraphicsItem: public OnHoverHandlerGraphicsItem
{
    Q_OBJECT

public:
    GasVertexHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                        QGraphicsItem* parent, int vertexNum)
        : OnHoverHandlerGraphicsItem(item, worldModel, parent, nullptr, nullptr, vertexNum) {}

protected:
    StepCore::Gas* gas() const;
    StepCore::Vector2d value() override;
    void setValue(const StepCore::Vector2d& value) override;
};

class GasGraphicsItem: public StepGraphicsItem {
public:
    GasGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void viewScaleChanged() override;
    void stateChanged() override;
    void worldDataChanged(bool) override;

protected:
    void mouseSetPos(const QPointF& pos, const QPointF&, MovingState) override;
    OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF& pos) override;
    StepCore::Gas* gas() const;

    //ArrowHandlerGraphicsItem *_centerHandler;
};

namespace Ui {
    class WidgetCreateGasParticles;
}

class GasCreationDialog;


class GasMenuHandler: public ItemMenuHandler
{
    Q_OBJECT

public:
    GasMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
        : ItemMenuHandler(object, worldModel, parent) {}

    void populateMenu(QMenu* menu, KActionCollection* actions) override;

public slots:
    void createGasParticles();

protected slots:
    bool createGasParticlesApply();
    void createGasParticlesCountChanged();
    void createGasParticlesConcentrationChanged();
    void clearGas();

protected:
    StepCore::Gas* gas() const;
    Ui::WidgetCreateGasParticles* _createGasParticlesUi;
    GasCreationDialog  *_creationDialog;

    friend class GasCreationDialog;
    static const int MAX_PARTICLES = 200;
//    bool                      _confChanged;
};

#endif

