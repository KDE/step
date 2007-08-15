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

#include "gasgraphics.h"

#include "worldmodel.h"
#include "worldfactory.h"
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QPainter>
#include <KLocale>

double GasCreator::random11()
{
    return double(qrand()) / (RAND_MAX/2) - 1;
}

bool GasCreator::sceneEvent(QEvent* event)
{
    if(event->type() == QEvent::GraphicsSceneMousePress) {
        _worldModel->simulationPause();

        _worldModel->beginMacro(i18n("Create %1", _className));
        _item = _worldModel->newItem(_className); Q_ASSERT(_item != NULL);
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);

        StepCore::Gas* gas = static_cast<StepCore::Gas*>(_item);
        _worldModel->newItem("GasLJForce", gas);
        StepCore::Object* ljforce = gas->items()[0];
        _worldModel->setProperty(ljforce, ljforce->metaObject()->property("depth"), 0.1);
        _worldModel->setProperty(ljforce, ljforce->metaObject()->property("rmin"), 0.1);

        for(int i=0; i<20; ++i) {
            _worldModel->newItem("GasParticle", gas);
        }
        for(int i=0; i<20; ++i) {
            StepCore::Object* obj = gas->items()[1+i];
            StepCore::Vector2d pos(random11(), random11());
            StepCore::Vector2d vel(random11()/4, random11()/4);
            _worldModel->setProperty( obj, obj->metaObject()->property("position"), QVariant::fromValue(pos) );
            _worldModel->setProperty( obj, obj->metaObject()->property("velocity"), QVariant::fromValue(vel) );
        }

        _worldModel->endMacro();
        event->accept();
        return true;
    }
    return false;

}
