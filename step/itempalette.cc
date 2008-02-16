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

#include <QAction>
#include <QEvent>
#include <QToolButton>
#include <QVBoxLayout>
#include <QActionGroup>
#include <QStyleOption>
#include <QScrollArea>
#include <QPainter>
#include <QtAlgorithms>

#include <KLocale>
#include <KIcon>

#include "itempalette.moc"

class QPaintEvent;

// Inspired by QToolBarSeparator
class Separator: public QWidget
{
public:
    explicit Separator(QWidget* parent): QWidget(parent) {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    }

    QSize sizeHint() const {
        QStyleOption opt; opt.initFrom(this);
        const int extent = style()->pixelMetric(QStyle::PM_ToolBarSeparatorExtent, &opt, parentWidget());
        return QSize(extent, extent);
    }

    void paintEvent(QPaintEvent *) {
        QPainter p(this); QStyleOption opt; opt.initFrom(this);
        style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &opt, &p, parentWidget());
    }
};

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

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    //_toolBar = new QToolBar(i18n("Palette"), this);
    //_toolBar->setOrientation(Qt::Vertical);
    //_toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    //_toolBar->setIconSize(QSize(22,22));
    //setWidget(_toolBar);

    _scrollArea = new QScrollArea(this);
    _scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _scrollArea->setFrameShape(QFrame::NoFrame);
    _scrollArea->setWidgetResizable(true);

    _widget = new QWidget(_scrollArea);
    _layout = new QVBoxLayout(_widget);
    _layout->setSpacing(0);

    _actionGroup = new QActionGroup(this);
    _actionGroup->setExclusive(true);

    _pointerAction = new QAction(i18n("Pointer"), this);
    _pointerAction->setToolTip(i18n("Selection pointer"));
    _pointerAction->setIcon(KIcon("pointer"));
    _pointerAction->setCheckable(true);
    _pointerAction->setChecked(true);
    _pointerAction->setProperty("step_object", "Pointer");
    _actionGroup->addAction(_pointerAction);

    QToolButton* pointer = new QToolButton(_widget);
    pointer->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    pointer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    pointer->setAutoRaise(true);
    pointer->setIconSize(QSize(22,22));
    pointer->setDefaultAction(_pointerAction);
    _layout->addWidget(pointer);
    //_widget->addAction(_pointerAction);
    //_toolBar->addAction(_pointerAction);
    //_toolBar->widgetForAction(_pointerAction)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //_toolBar->addSeparator();

    foreach(QString name, _worldModel->worldFactory()->paletteMetaObjects()) {
        if(name.isEmpty()) _layout->addWidget(new Separator(_widget)); // _toolBar->addSeparator();
        else addObject(_worldModel->worldFactory()->metaObject(name));
    }

    _layout->addStretch();

    //_widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    _scrollArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    _scrollArea->setWidget(_widget);
    setWidget(_scrollArea);

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
    action->setIcon(_worldModel->worldFactory()->objectIcon(metaObject));
    action->setCheckable(true);
    action->setProperty("step_object", metaObject->className());

    _actionGroup->addAction(action);
    QToolButton* button = new QToolButton(_widget);
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    button->setAutoRaise(true);
    button->setIconSize(QSize(22,22));
    button->setDefaultAction(action);
    _layout->addWidget(button);
    //_widget->addAction(action);
    //_toolBar->addAction(action);
    //_toolBar->widgetForAction(action)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void ItemPalette::actionTriggered(QAction* action)
{
    emit beginAddItem(action->property("step_object").toString());
}

void ItemPalette::endAddItem(const QString& name, bool /*success*/)
{
    if(name == _actionGroup->checkedAction()->property("step_object").toString())
        _pointerAction->setChecked(true);
}

bool ItemPalette::event(QEvent* event)
{
    return QDockWidget::event(event);
}

