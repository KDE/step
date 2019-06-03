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

class WorldModel;
class QToolButton;
class QScrollArea;
class QAction;
class QActionGroup;
class PaletteLayout;

namespace StepCore { class MetaObject; }

class ItemPalette: public QDockWidget
{
    Q_OBJECT

public:
    explicit ItemPalette(WorldModel* worldModel, QWidget* parent = 0);

signals:
    void beginAddItem(const QString& name);

public slots:
    void endAddItem(const QString& name, bool success);

protected slots:
    void actionTriggered(QAction* action);
    void showButtonTextToggled(bool b);

protected:
    void createSeparator();
    void createToolButton(QAction* action);
    void createObjectAction(const StepCore::MetaObject* metaObject);

    bool event(QEvent* event) Q_DECL_OVERRIDE;

    WorldModel*     _worldModel;
    QScrollArea*    _scrollArea;
    QWidget*        _widget;
    PaletteLayout*  _layout;

    QAction*        _pointerAction;
    QActionGroup*   _actionGroup;

    QList<QToolButton*> _toolButtons;
};

#endif

