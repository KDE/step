/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "worldbrowser.h"

#include "worldmodel.h"
#include <stepcore/world.h>

#include <QApplication>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QTreeView>

#include <KLocalizedString>

class WorldBrowserView: public QTreeView
{
public:
    WorldBrowserView(QWidget* parent = nullptr);
    void reset() override;

protected:
    void changeEvent(QEvent* event) override;
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const override;
    void keyPressEvent(QKeyEvent* e) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
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

WorldBrowser::WorldBrowser(WorldModel* worldModel, QWidget* parent)
    : QDockWidget(i18nc("Object list", "World"), parent)
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
    QStyleOptionViewItem opt;
    initViewItemOption(&opt);

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

#include "moc_worldbrowser.cpp"
