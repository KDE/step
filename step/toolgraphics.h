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
#include <QTextEdit>

class NoteCreator: public ItemCreator
{
public:
    NoteCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);
};

class NoteWidgetItem: public QGraphicsItem
{
public:
    NoteWidgetItem(QGraphicsItem *parent = 0);
    ~NoteWidgetItem();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

protected:
    //bool eventFilter(QObject *watched, QEvent *event);

private:
    void adjust();
    QTextEdit* _textEdit;
};

class NoteGraphicsItem: public WorldGraphicsItem {
public:
    NoteGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void advance(int phase);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    void mouseSetPos(const QPointF& pos, const QPointF& diff);
    StepCore::Note* note() const;
    NoteWidgetItem* _widgetItem;
    QGraphicsTextItem* _textItem;

    /*
    double _rnorm;
    double _rscale;
    double _radius;

    SpringHandlerGraphicsItem* _handler1;
    SpringHandlerGraphicsItem* _handler2;

    static const int RADIUS = 6;
    friend class NoteCreator;
    */
};

#endif

