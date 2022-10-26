/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
    SPDX-FileCopyrightText: 2014 Inge Wallin <inge@lysator.liu.se>

    SPDX-License-Identifier: GPL-2.0-or-later
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

