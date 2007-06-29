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

#ifndef STEP_TOOLGRAPHICS_H
#define STEP_TOOLGRAPHICS_H

#include "worldgraphics.h"
#include <stepcore/tool.h>
#include <QGraphicsTextItem>
#include <QAbstractItemModel>
#include <QWidget>

class NoteGraphicsItem;
class NoteTextItem: public QGraphicsTextItem
{
public:
    NoteTextItem(NoteGraphicsItem* noteItem, QGraphicsItem* parent = 0);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QString emptyNotice() const;

protected:
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    NoteGraphicsItem* _noteItem;
};

class NoteGraphicsItem: public QObject, public WorldGraphicsItem
{
    Q_OBJECT

public:
    NoteGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void advance(int phase);

protected slots:
    void contentsChanged();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    StepCore::Note* note() const;
    NoteTextItem*   _textItem;
    bool            _updating;
    double          _lastScale;

    friend class NoteTextItem;
};

class KPlotWidget;
class QComboBox;
class QModelIndex;
class GraphGraphicsItem;
class QLabel;

class GraphFlatWorldModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    GraphFlatWorldModel(WorldModel* worldModel, QObject* parent = 0);
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

    QModelIndex parent(const QModelIndex &) const { return QModelIndex(); }
    int columnCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent); return 1; }
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role) const;

protected slots:
    void sourceDataChanged(const QModelIndex&, const QModelIndex&) {
        emit dataChanged(index(0, 0), index(rowCount(), 0));
    }
    void sourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end) {
        beginInsertRows(QModelIndex(), mapRow(parent, start), mapRow(parent, end));
    }
    void sourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end) {
        beginRemoveRows(QModelIndex(), mapRow(parent, start), mapRow(parent, end));
    }
    void sourceRowsInserted(const QModelIndex&, int, int) { endInsertRows(); }
    void sourceRowsRemoved(const QModelIndex&, int, int) { endRemoveRows(); }

protected:
    int mapRow(const QModelIndex& sourceParent, int sourceRow);
    WorldModel* _worldModel;
};

class GraphWidget: public QWidget
{
    Q_OBJECT

public:
    GraphWidget(GraphGraphicsItem* graphItem, QWidget *parent = 0);
    ~GraphWidget();

    void advance();

protected slots:
    void objectSelected(const QString& text);

protected:
    GraphGraphicsItem* _graphItem;
    KPlotWidget*       _plotWidget;
    QLabel*     _name;
    QComboBox*  _object1;
    QComboBox*  _property1;
    QComboBox*  _index1;
    QComboBox*  _object2;
    QComboBox*  _property2;
    QComboBox*  _index2;
    bool        _updating;
};

class GraphGraphicsItem: public WorldGraphicsItem
{
public:
    GraphGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    ~GraphGraphicsItem();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void advance(int phase);

protected:
    StepCore::Graph* graph() const;

    double _lastScale;
    GraphWidget *_graphWidget;
    //QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    //void mouseSetPos(const QPointF& pos, const QPointF& diff);
    
    //NoteTextItem*   _textItem;
    //bool            _updating;
    //double          _lastScale;

    friend class GraphWidget;
};


#endif

