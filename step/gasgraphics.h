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

#ifndef STEP_GASGRAPHICS_H
#define STEP_GASGRAPHICS_H

#include "worldgraphics.h"

namespace StepCore {
    class Gas;
}

class GasCreator: public ItemCreator
{
public:
    GasCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
           : ItemCreator(className, worldModel, worldScene) {}

    bool sceneEvent(QEvent* event);
};

class GasArrowHandlerGraphicsItem;
class GasGraphicsItem: public WorldGraphicsItem {
public:
    GasGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void viewScaleChanged();
    void stateChanged();
    void worldDataChanged(bool);

protected:
    StepCore::Gas* gas() const;

    ArrowHandlerGraphicsItem *_centerHandler;
    GasArrowHandlerGraphicsItem *_sizeHandler;
};

namespace Ui {
    class WidgetCreateGasParticles;
}

class GasKDialog;
class GasMenuHandler: public ItemMenuHandler
{
    Q_OBJECT

public:
    GasMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
        : ItemMenuHandler(object, worldModel, parent) {}

    void populateMenu(QMenu* menu);

protected slots:
    void createGasParticles();
    bool createGasParticlesApply();
    void createGasParticlesCountChanged();
    void createGasParticlesConcentrationChanged();
    void clearGas();

protected:
    StepCore::Gas* gas() const;
    Ui::WidgetCreateGasParticles* _createGasParticlesUi;
    GasKDialog*                   _createGasParticlesDialog;
    friend class GasKDialog;
    static const int MAX_PARTICLES = 200;
//    bool                      _confChanged;
};

#endif

