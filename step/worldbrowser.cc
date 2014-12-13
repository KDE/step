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

#include "worldbrowser.h"

#include "worldmodel.h"
#include <stepcore/world.h>
#include <QTreeView>
#include <QHeaderView>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QApplication>
#include <QMenu>
#include <KLocale>

class WorldBrowserView: public QTreeView
{
public:
    WorldBrowserView(QWidget* parent = 0);
    virtual void reset();

protected:
    void changeEvent(QEvent* event);
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const;
    void keyPressEvent(QKeyEvent* e);
    void contextMenuEvent(QContextMenuEvent* event);
    WorldModel* worldModel() { return static_cast<WorldModel*>(model()); }
    const int _windowsDecoSize;
    bool _macStyle;
};

WorldBrowserView::WorldBrowserView(QWidget* parent)
        : QTreeView(parent), _windowsDecoSize(9)
{
    _macStyle = QApplication::style()->inherits("QMacStyle");
}

void WorldBrowserView::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::StyleChange)
        _macStyle = QApplication::style()->inherits("QMacStyle");
}

WorldBrowser::WorldBrowser(WorldModel* worldModel, QWidget* parent, Qt::WindowFlags flags)
    : QDockWidget(i18nc("Object list", "World"), parent, flags)
{
    _worldBrowserView = new WorldBrowserView(this);
    _worldBrowserView->header()->hide();
    _worldBrowserView->setAllColumnsShowFocus(true);
    _worldBrowserView->setRootIsDecorated(false);
    _worldBrowserView->setModel(worldModel);
    _worldBrowserView->setSelectionModel(worldModel->selectionModel());
    _worldBrowserView->setSelectionMode(QAbstractItemView::ExtendedSelection); // XXX
    setWidget(_worldBrowserView);
}

void WorldBrowserView::reset()
{
    QTreeView::reset();
    expand(worldModel()->worldIndex());
}

void WorldBrowserView::keyPressEvent(QKeyEvent* e)
{
    if(e->matches(QKeySequence::Delete)) {
        worldModel()->deleteSelectedItems();
        e->accept();
    } else QTreeView::keyPressEvent(e);
}

void WorldBrowserView::contextMenuEvent(QContextMenuEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    if(!index.isValid()) return;

    event->accept();
    QMenu* menu = worldModel()->createContextMenu(index);
    menu->exec(event->globalPos());
    delete menu;
}

void WorldBrowserView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    // Inspired by qt-designer code in src/components/propertyeditor/qpropertyeditor.cpp
    if(!index.parent().isValid()) return;
    QStyleOptionViewItem opt = viewOptions();

    if(model()->hasChildren(index)) {
        opt.state |= QStyle::State_Children;

        QRect primitive(rect.left() + rect.width() - indentation(), rect.top(),
                                                    indentation(), rect.height());

        if (!_macStyle) {
            primitive.moveLeft(primitive.left() + (primitive.width() - _windowsDecoSize)/2);
            primitive.moveTop(primitive.top() + (primitive.height() - _windowsDecoSize)/2);
            primitive.setWidth(_windowsDecoSize);
            primitive.setHeight(_windowsDecoSize);
        }

        opt.rect = primitive;

        if(isExpanded(index)) opt.state |= QStyle::State_Open;
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
    }
}

