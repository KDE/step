/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STEP_PARTICLEGRAPHICS_H
#define STEP_PARTICLEGRAPHICS_H

#include "worldgraphics.h"
#include "stepgraphicsitem.h"

#include <stepcore/particle.h>

class ParticleGraphicsItem: public StepGraphicsItem {
public:
    ParticleGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void viewScaleChanged() override;
    void stateChanged() override;
    void worldDataChanged(bool) override;

protected:
    StepCore::Particle* particle() const {
        return static_cast<StepCore::Particle*>(_item); }

    ArrowHandlerGraphicsItem *_velocityHandler;
    double _lastArrowRadius;

    static const int RADIUS = 7;
};

#endif

