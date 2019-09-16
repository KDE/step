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

#include "worldgraphics.h"

#include "settings.h"

#include "worldmodel.h"
#include "stepgraphicsitem.h"
#include <stepcore/object.h>
#include <stepcore/world.h>
#include <stepcore/particle.h>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QItemSelectionModel>
#include <QMenu>

#include <KActionCollection>
#include <KLocalizedString>

#include <cmath>

// XXX
#include "worldscene.h"

void ItemCreator::showMessage(MessageFrame::Type type, const QString& text, MessageFrame::Flags flags)
{
    if(Settings::showCreationTips()) {
        if(!(flags & MessageFrame::CloseButton) && !(flags & MessageFrame::CloseTimer)) {
            _messageId = _worldScene->changeMessage(_messageId, type, text, flags);
        } else {
            _worldScene->showMessage(type, text, flags);
        }
    }
}

void ItemCreator::closeMessage()
{
    _worldScene->closeMessage(_messageId);
}

void ItemCreator::start()
{
    showMessage(MessageFrame::Information,
            i18n("Click on the scene to create a %1", classNameTr()));
}

bool ItemCreator::sceneEvent(QEvent* event)
{
    if(event->type() == QEvent::GraphicsSceneMousePress) {
        _worldModel->simulationPause();

        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->createItem(_className); Q_ASSERT(_item != NULL);
        const StepCore::MetaProperty* property = _item->metaObject()->property(QStringLiteral("position"));
        if(property != NULL) {
            QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
            QPointF pos = mouseEvent->scenePos();
            QVariant vpos = QVariant::fromValue(StepGraphicsItem::pointToVector(pos));
            _worldModel->setProperty(_item, property, vpos);
        }
        _worldModel->addItem(_item);

        _worldModel->endMacro();

        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);

        showMessage(MessageFrame::Information,
                i18n("%1 named '%2' created", classNameTr(), _item->name()),
                MessageFrame::CloseButton | MessageFrame::CloseTimer);

        setFinished();
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

void AttachableItemCreator::start()
{
    if(_twoEnds)
        showMessage(MessageFrame::Information,
            i18n("Press left mouse button to position first end of a %1\n"
                 "then drag and release it to position the second end", classNameTr()));
    else
        showMessage(MessageFrame::Information,
            i18n("Click on the scene to create a %1", classNameTr()));
}

bool AttachableItemCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

    if(event->type() == QEvent::GraphicsSceneMouseMove && _item == NULL) {
        _worldScene->snapHighlight(mouseEvent->scenePos(), _snapFlags, _snapTypes);
        return false;

    } else if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->createItem(className()); Q_ASSERT(_item != NULL);

        if(_twoEnds) {
            _worldScene->snapItem(pos, _snapFlags, _snapTypes, StepGraphicsItem::Finished, _item, 1);
            _worldModel->setProperty(_item, QStringLiteral("localPosition2"),
                            QVariant::fromValue(StepGraphicsItem::pointToVector(pos)));
            _worldModel->setProperty(_item, QStringLiteral("restLength"), 0);

            showMessage(MessageFrame::Information,
                i18n("Release left mouse button to position second end of the %1", classNameTr()));
        } else {
            _worldScene->snapItem(pos, _snapFlags, _snapTypes, StepGraphicsItem::Finished, _item);
            showMessage(MessageFrame::Information,
                i18n("%1 named '%2' created", classNameTr(), _item->name()),
                MessageFrame::CloseButton | MessageFrame::CloseTimer);
            _worldModel->endMacro();
            setFinished();
        }
        _worldModel->addItem(_item);
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                       QItemSelectionModel::ClearAndSelect);
        return true;

    } else if(event->type() == QEvent::GraphicsSceneMouseMove &&
                    (mouseEvent->buttons() & Qt::LeftButton) && _twoEnds) {

        QPointF pos = mouseEvent->scenePos();
        _worldScene->snapItem(pos, _snapFlags, _snapTypes, StepGraphicsItem::Moving, _item, 2);

        double length =
            (_item->metaObject()->property(QStringLiteral("position2"))->readVariant(_item).value<StepCore::Vector2d>() -
             _item->metaObject()->property(QStringLiteral("position1"))->readVariant(_item).value<StepCore::Vector2d>()).norm();
        _worldModel->setProperty(_item, QStringLiteral("restLength"), length);
        return true;

    } else if(event->type() == QEvent::GraphicsSceneMouseRelease &&
                    (mouseEvent->button() == Qt::LeftButton) && _twoEnds) {

        QPointF pos = mouseEvent->scenePos();
        _worldScene->snapItem(pos, _snapFlags, _snapTypes, StepGraphicsItem::Finished, _item, 2);

        double length =
            (_item->metaObject()->property(QStringLiteral("position2"))->readVariant(_item).value<StepCore::Vector2d>() -
             _item->metaObject()->property(QStringLiteral("position1"))->readVariant(_item).value<StepCore::Vector2d>()).norm();
        _worldModel->setProperty(_item, QStringLiteral("restLength"), length);
        _worldModel->endMacro();

        showMessage(MessageFrame::Information,
            i18n("%1 named '%2' created", classNameTr(), _item->name()),
            MessageFrame::CloseButton | MessageFrame::CloseTimer);

        setFinished();
        return true;
    }

    return false;
}

void AttachableItemCreator::abort()
{
    _worldScene->snapClear();
}

/////////////////////////////////////////////////////////////////////////////////////////

ItemMenuHandler::ItemMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
    : QObject(parent)
    , _object(object)
    , _worldModel(worldModel)
{
}

void ItemMenuHandler::populateMenu(QMenu* menu, KActionCollection* actions)
{
    StepCore::Item* item = dynamic_cast<StepCore::Item*>(_object);
    
    if (item && item->world() != item) {
        menu->addAction(actions->action(QStringLiteral("edit_cut")));
        menu->addAction(actions->action(QStringLiteral("edit_copy")));
        menu->addAction(actions->action(QStringLiteral("edit_delete")));
    }
}
