/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
    SPDX-FileCopyrightText: 2014 Inge Wallin <inge@lysator.liu.se>

    SPDX-License-Identifier: GPL-2.0-or-later
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

    bool sceneEvent(QEvent* event) override;
    void start() override;
    
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class QDialog;
class SoftBodyMenuHandler: public ItemMenuHandler
{
    Q_OBJECT

public:
    SoftBodyMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
        : ItemMenuHandler(object, worldModel, parent), _applied(false) {}

    void populateMenu(QMenu* menu, KActionCollection* actions) override;

    bool applied() const { return _applied; }

public slots:
    void createSoftBodyItems(const StepCore::Vector2d& pos);

protected slots:
    void createSoftBodyItemsApply();
    void clearSoftBody();

protected:
    StepCore::SoftBody* softBody() const;
    Ui::WidgetCreateSoftBodyItems* _createSoftBodyItemsUi;
    QDialog*                       _createSoftBodyItemsDialog;
    bool                           _applied;
//    bool                      _confChanged;
};

class SoftBodyGraphicsItem: public StepGraphicsItem
{
public:
    SoftBodyGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void viewScaleChanged() override;
    void stateChanged() override;
    void worldDataChanged(bool dynamicOnly) override;

protected:
    void mouseSetPos(const QPointF& pos, const QPointF& diff, MovingState) override;
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

    void worldDataChanged(bool dynamicOnly) override;
};

class SoftBodySpringGraphicsItem: public SpringGraphicsItem
{
public:
    SoftBodySpringGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
        : SpringGraphicsItem(item, worldModel) {}

    void worldDataChanged(bool dynamicOnly) override;
};

#endif

