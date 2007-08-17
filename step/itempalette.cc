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

#include "itempalette.h"

#include "worldmodel.h"
#include "worldfactory.h"
#include <stepcore/world.h>

#include <QToolBox>
#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QtAlgorithms>

#include <KLocale>
#include <KIcon>

#include "itempalette.moc"

ItemPalette::ItemPalette(WorldModel* worldModel, QWidget* parent, Qt::WindowFlags flags)
    : QDockWidget(i18n("Palette"), parent, flags), _worldModel(worldModel)
{
    /*
    _toolBox = new QToolBox(this);
    setWidget(_toolBox);

    _bodiesToolBar = new QToolBar("Bodies", _toolBox);
    _forcesToolBar = new QToolBar("Forces", _toolBox);

    _bodiesToolBar->setOrientation( Qt::Vertical );
    _forcesToolBar->setOrientation( Qt::Vertical );

    _toolBox->addItem(_bodiesToolBar, "Bodies");
    _toolBox->addItem(_forcesToolBar, "Forces");

    _actionGroup = new QActionGroup(this);
    _actionGroup->setExclusive(true);

    _pointerAction = new QAction("Pointer", this);
    _pointerAction->setCheckable(true);
    _pointerAction->setChecked(true);
    _actionGroup->addAction(_pointerAction);
    _bodiesToolBar->addAction(_pointerAction);
    _forcesToolBar->addAction(_pointerAction);
    _bodiesToolBar->addSeparator();
    _forcesToolBar->addSeparator();
    */

    _toolBar = new QToolBar(i18n("Palette"), this);
    _toolBar->setOrientation(Qt::Vertical);
    _toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _toolBar->setIconSize(QSize(22,22));
    setWidget(_toolBar);

    _actionGroup = new QActionGroup(this);
    _actionGroup->setExclusive(true);

    _pointerAction = new QAction(i18n("Pointer"), this);
    _pointerAction->setToolTip(i18n("Selection pointer"));
    _pointerAction->setIcon(KIcon("pointer"));
    _pointerAction->setCheckable(true);
    _pointerAction->setChecked(true);
    _actionGroup->addAction(_pointerAction);
    _toolBar->addAction(_pointerAction);
    _toolBar->widgetForAction(_pointerAction)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _toolBar->addSeparator();

    foreach(QString name, _worldModel->worldFactory()->paletteMetaObjects()) {
        if(name.isEmpty()) _toolBar->addSeparator();
        else addObject(_worldModel->worldFactory()->metaObject(name));
    }

#if 0
    /* Add bodies */
    foreach(QString name, _worldModel->worldFactory()->orderedMetaObjects()) {
        const StepCore::MetaObject* metaObject = _worldModel->worldFactory()->metaObject(name);
        if(metaObject->isAbstract()) continue;
        if(!metaObject->inherits(StepCore::Body::staticMetaObject())) continue;
        addObject(metaObject);
    }

    /* Add groups */
    _toolBar->addSeparator();
    foreach(QString name, _worldModel->worldFactory()->orderedMetaObjects()) {
        const StepCore::MetaObject* metaObject = _worldModel->worldFactory()->metaObject(name);
        if(metaObject->isAbstract()) continue;
        if(!metaObject->inherits(StepCore::ItemGroup::staticMetaObject())) continue;
        if(metaObject == StepCore::World::staticMetaObject() ||
            metaObject == StepCore::ItemGroup::staticMetaObject()) continue;
        addObject(metaObject);
    }

    /* Add forces */
    _toolBar->addSeparator();
    foreach(QString name, _worldModel->worldFactory()->orderedMetaObjects()) {
        const StepCore::MetaObject* metaObject = _worldModel->worldFactory()->metaObject(name);
        if(metaObject->isAbstract()) continue;
        if(!metaObject->inherits(StepCore::Force::staticMetaObject())) continue;
        addObject(metaObject);
    }

    /* Add tools */
    _toolBar->addSeparator();
    foreach(QString name, _worldModel->worldFactory()->orderedMetaObjects()) {
        const StepCore::MetaObject* metaObject = _worldModel->worldFactory()->metaObject(name);
        if(metaObject->isAbstract()) continue;
        if(!metaObject->inherits(StepCore::Tool::staticMetaObject())) continue;
        addObject(metaObject);
    }
#endif

    QObject::connect(_actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));
}

void ItemPalette::addObject(const StepCore::MetaObject* metaObject)
{
    Q_ASSERT(metaObject && !metaObject->isAbstract());

    QAction* action = new QAction(metaObject->className(), this);
    action->setToolTip(metaObject->description());
    action->setIcon(KIcon(QString("step_object_") + metaObject->className()));
    action->setCheckable(true);

    _actionGroup->addAction(action);
    _toolBar->addAction(action);
    _toolBar->widgetForAction(action)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void ItemPalette::actionTriggered(QAction* action)
{
    emit beginAddItem(action->text());
}

void ItemPalette::endAddItem(const QString& name, bool /*success*/)
{
    if(name == _actionGroup->checkedAction()->text())
        _pointerAction->setChecked(true);
}

