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

#ifndef STEP_ITEMPALETTE_H
#define STEP_ITEMPALETTE_H

#include <QDockWidget>
#include <QList>
#include <QHash>

class WorldModel;
class QVBoxLayout;
class QToolButton;
class QScrollArea;
class QAction;
class QActionGroup;
class PaletteLayout;

namespace StepCore {
    class MetaObject;
    class World;
    class Item;
}

class ItemPalette: public QDockWidget
{
    Q_OBJECT

public:
    explicit ItemPalette(WorldModel* worldModel, QWidget* parent = 0, Qt::WindowFlags flags = 0);
    ~ItemPalette();

signals:
    void beginAddItem(const QString& name, const StepCore::Item* item);

public slots:
    void endAddItem(const QString& name, bool success, bool selectPointer);

protected slots:
    void actionTriggered(QAction* action);
    void showButtonTextToggled(bool b);
    void groupVisibilityToggled(QAction* action);
    void configureCustomGroups();

protected:
    QAction* createSeparator();
    QAction* createObjectAction(const StepCore::MetaObject* metaObject);
    QAction* createCustomItemAction(const QString& filename, const StepCore::Item* item);
    void createToolButton(QAction* action);

    void loadCustomGroup(const QString& filename);

    void updateGroupsVisibility();

    bool event(QEvent* event);

    WorldModel*     _worldModel;
    QScrollArea*    _scrollArea;
    QWidget*        _widget;
    PaletteLayout*  _layout;

    QAction*        _pointerAction;
    QActionGroup*   _actionGroup;
    QActionGroup*   _groupsActionGroup;

    QList<QToolButton*> _toolButtons;

    QHash<QString, QList<QAction*> > _groups;
    QHash<QString, StepCore::World* > _groupWorlds;
};

#endif

