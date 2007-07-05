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
#include <QComboBox>
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

    void viewScaleChanged();
    void worldDataChanged(bool dynamicOnly);

protected slots:
    void contentsChanged();

protected:
    StepCore::Note* note() const;
    NoteTextItem*   _textItem;
    int             _updating;
    double          _lastScale;

    friend class NoteTextItem;
};

class KPlotWidget;
class KPlotObject;
class KAction;
class QLabel;

class DataSourceWidget: public QWidget
{
    Q_OBJECT

public:
    DataSourceWidget(QWidget* parent = 0);

    void setDataSource(WorldModel* worldModel, const QString& object = QString(),
                            const QString& property = QString(), int index = 0);

    QString dataObject() const { return _object->currentText(); }
    QString dataProperty() const { return _property->currentText(); }
    int dataIndex() const { return _index->currentIndex(); }

signals:
    void dataSourceChanged();

protected slots:
    void objectSelected(const QString& text);
    void propertySelected(const QString& text);

protected:
    WorldModel* _worldModel;

    QComboBox*  _object;
    QComboBox*  _property;
    QComboBox*  _index;
};

/*
class GraphWidget: public QWidget
{
    Q_OBJECT

public:
    GraphWidget(GraphGraphicsItem* graphItem, QWidget *parent = 0);
    ~GraphWidget();

    void worldDataChanged();

protected slots:
    void objectSelected(const QString& text);
    void propertySelected(const QString& text);
    void indexSelected(const QString& text);
    void recordPoint();
    void configure();

protected:
    GraphGraphicsItem* _graphItem;
    KPlotWidget*       _plotWidget;
    KPlotObject*       _plotObject;
    QLabel*     _name;
    QComboBox*  _object[2];
    QComboBox*  _property[2];
    QComboBox*  _index[2];

    StepCore::Graph* _graph;
    WorldModel* _worldModel;
    int         _updating;
    int         _doclear;

    double      _lastPointTime;

    KAction*    _configureAction;
    KAction*    _clearAction;

    friend class GraphGraphicsItem;
};*/

namespace Ui { class WidgetConfigureGraph; }
class KDialog;

class GraphGraphicsItem: public QObject, public WorldGraphicsItem
{
    Q_OBJECT

public:
    GraphGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    ~GraphGraphicsItem();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void stateChanged();
    void viewScaleChanged();
    void worldDataChanged(bool);

protected slots:
    void configure();
    void confApply();
    void confChanged();

protected:
    StepCore::Graph* graph() const;

    double _lastScale;
    double _lastPointTime;

    KPlotWidget *_plotWidget;
    KAction* _clearAction;
    KAction* _configureAction;

    Ui::WidgetConfigureGraph* _confUi;
    KDialog*                  _confDialog;
    //QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    //void mouseSetPos(const QPointF& pos, const QPointF& diff);
    
    //NoteTextItem*   _textItem;
    //bool            _updating;
    //double          _lastScale;

    //friend class GraphWidget;
};


#endif

