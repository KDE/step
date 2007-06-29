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

#include "toolgraphics.h"
#include "toolgraphics.moc"

#include <stepcore/solver.h>
#include <stepcore/collisionsolver.h>

#include "worldmodel.h"
#include "worldfactory.h"
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTextDocument>
#include <QEvent>
#include <QPainter>
#include <QGridLayout>
#include <QComboBox>
#include <QLabel>
#include <KPlotWidget>
#include <KLocale>

NoteTextItem::NoteTextItem(NoteGraphicsItem* noteItem, QGraphicsItem* parent)
    : QGraphicsTextItem(parent), _noteItem(noteItem)
{
    setPlainText(emptyNotice());
}

QString NoteTextItem::emptyNotice() const
{
    return i18n("Click to enter a text");
}

void NoteTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem opt = *option; // XXX: are there any documented way to do this ?
    if(_noteItem->isSelected()) opt.state |=  QStyle::State_HasFocus;
    QGraphicsTextItem::paint(painter, &opt, widget);
}

void NoteTextItem::focusInEvent(QFocusEvent *event)
{
    if(_noteItem->note()->text().isEmpty()) {
        _noteItem->_updating = true;
        setPlainText("");
        _noteItem->advance(1);
        _noteItem->_updating = false;
    }
    QGraphicsTextItem::focusInEvent(event);
}

void NoteTextItem::focusOutEvent(QFocusEvent *event)
{
    if(_noteItem->note()->text().isEmpty()) {
        _noteItem->_updating = true;
        setPlainText(emptyNotice());
        _noteItem->advance(1);
        _noteItem->_updating = false;
    }
    QGraphicsTextItem::focusOutEvent(event);
}

NoteGraphicsItem::NoteGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Note*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);

    _textItem = new NoteTextItem(this, this);
    _textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
    _textItem->scale(1, -1);
    _lastScale = 1;
    _updating = false;
    connect(_textItem->document(), SIGNAL(contentsChanged()), this, SLOT(contentsChanged()));
    advance(1);
}

inline StepCore::Note* NoteGraphicsItem::note() const
{
    return static_cast<StepCore::Note*>(_item);
}

void NoteGraphicsItem::paint(QPainter* /*painter*/, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    /*painter->setPen(QPen(Qt::black, 0));
    painter->setBrush(QBrush(Qt::lightGray));
    QRectF rect = boundingRect();
    painter->drawRect(rect);*/
    /*
    double s = currentViewScale();
    painter->setPen(QPen(Qt::gray, 0));
    painter->drawLine(QLineF(0, -4/s, rect.width(), -4/s)); 
    painter->drawLine(QLineF(0, -8/s, rect.width(), -8/s)); 
    */

    /*
    double s = currentViewScale();
    double radius = 6/s;

    int renderHints = painter->renderHints();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::black, 0));
    painter->setBrush(QBrush(Qt::black));
    
    painter->drawEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    painter->setBrush(QBrush());
    painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);

    if(isSelected()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        //painter->setBrush(QBrush(QColor(0, 0x99, 0xff)));
        radius = (6+SELECTION_MARGIN)/s;
        painter->drawEllipse(QRectF(-radius, -radius, radius*2, radius*2));
        painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);
    }*/
}

void NoteGraphicsItem::advance(int phase)
{
    if(phase == 0) return;

    _worldModel->simulationPause();
    if(!_updating && _textItem->toPlainText() != note()->text()) {
        _updating = true;
        if(!_textItem->hasFocus() && note()->text().isEmpty()) {
            _textItem->setPlainText(_textItem->emptyNotice());
        } else {
            _textItem->setPlainText(note()->text());
        }
        _updating = false;
    }

    double s = currentViewScale();
    if(s != _lastScale) {
        _textItem->resetTransform();
        _textItem->scale(1/s, -1/s);
        _lastScale = s;
    }
    
    QPointF r = vectorToPoint(note()->position());
    QSizeF  size = _textItem->boundingRect().size()/s;
    size.setHeight(-size.height());

    if(size != _boundingRect.size()) {
        prepareGeometryChange();
        _boundingRect.setSize(size);
    }
    
    if(r != pos()) setPos(r);
    update(); // XXX: documentation says this is unnessesary, but it doesn't work without it
}

void NoteGraphicsItem::contentsChanged()
{
    if(!_updating) {
        _updating = true;
        _worldModel->simulationPause();
        _worldModel->setProperty(_item, _item->metaObject()->property("text"),
                                QVariant::fromValue( _textItem->toPlainText() ));
        _updating = false;
    }
}

QVariant NoteGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    /*
    if(change == QGraphicsItem::ItemSelectedChange && scene()) {
        if(value.toBool()) {
            _velocityHandler->setVisible(true);
        } else {
            _velocityHandler->setVisible(false);
        }
    }*/
    return WorldGraphicsItem::itemChange(change, value);
}


QModelIndex GraphFlatWorldModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!parent.isValid()) return createIndex(row, column);
    else return QModelIndex();
}

int GraphFlatWorldModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid()) return _worldModel->rowCount() + _worldModel->itemCount();
    else return 0;
}

int GraphFlatWorldModel::mapRow(const QModelIndex& sourceParent, int sourceRow)
{
    if(sourceParent.isValid()) {
        return sourceRow + 1;
    } else {
        if(sourceRow > 0) return sourceRow + _worldModel->itemCount();
        else return sourceRow;
    }
}

QVariant GraphFlatWorldModel::data(const QModelIndex &index, int role) const
{
    if(index.isValid() && role == Qt::DisplayRole) {
        if(index.row() == 0) return _worldModel->world()->name();
        if(index.row() < _worldModel->itemCount()+1)
            return _worldModel->item(index.row()-1)->name();
        else return _worldModel->object(_worldModel->index(
                        index.row()-_worldModel->itemCount(), 0))->name();
    }
    return QVariant();
}

GraphFlatWorldModel::GraphFlatWorldModel(WorldModel* worldModel, QObject* parent)
    : QAbstractItemModel(parent), _worldModel(worldModel)
{
    connect(_worldModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
    connect(_worldModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
    connect(_worldModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
    connect(_worldModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(_worldModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));
    connect(_worldModel, SIGNAL(modelReset()), this, SIGNAL(modelReset()));
}

GraphWidget::GraphWidget(GraphGraphicsItem* graphItem, QWidget *parent)
    : QWidget(parent), _graphItem(graphItem)
{
    _updating = false;

    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setColumnStretch(0, 0);
    gridLayout->setColumnStretch(1, 5);
    gridLayout->setColumnStretch(2, 5);
    gridLayout->setColumnStretch(3, 1);

    gridLayout->setRowStretch(0, 0);
    gridLayout->setRowStretch(1, 1);
    gridLayout->setRowStretch(2, 0);
    gridLayout->setRowStretch(3, 0);

    _name = new QLabel(_graphItem->graph()->name(), this);
    _name->setAlignment(Qt::AlignHCenter);
    QFont font = _name->font(); font.setBold(true); _name->setFont(font);
    gridLayout->addWidget(_name, 0, 0, 1, -1);

    _plotWidget = new KPlotWidget(this);
    _plotWidget->setBackgroundColor(Qt::white);
    _plotWidget->setForegroundColor(Qt::black);
    //_plotWidget->setLeftPadding(0);
    _plotWidget->setTopPadding(2);
    _plotWidget->setRightPadding(3);
    gridLayout->addWidget(_plotWidget, 1, 0, 1, -1);

    QLabel* label1 = new QLabel("x:", this);
    gridLayout->addWidget(label1, 2, 0, 1, 1);

    _object1 = new QComboBox(this);
    gridLayout->addWidget(_object1, 2, 1, 1, 1);

    _property1 = new QComboBox(this);
    gridLayout->addWidget(_property1, 2, 2, 1, 1);

    _index1 = new QComboBox(this);
    gridLayout->addWidget(_index1, 2, 3, 1, 1);

    QLabel* label2 = new QLabel("y:", this);
    gridLayout->addWidget(label2, 3, 0, 1, 1);

    _object2 = new QComboBox(this);
    gridLayout->addWidget(_object2, 3, 1, 1, 1);

    _property2 = new QComboBox(this);
    gridLayout->addWidget(_property2, 3, 2, 1, 1);

    _index2 = new QComboBox(this);
    gridLayout->addWidget(_index2, 3, 3, 1, 1);

    _object1->setModel(new GraphFlatWorldModel(_graphItem->_worldModel, this));
    //_object1->setRootModelIndex(_graphItem->_worldModel->worldIndex());
    _object2->setModel(_object1->model());

    connect(_object1, SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(objectSelected(const QString&)));
    connect(_object2, SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(objectSelected(const QString&)));
}

GraphWidget::~GraphWidget()
{
    _plotWidget->hide(); // BUG ?
    delete _plotWidget;
}

void GraphWidget::objectSelected(const QString& text)
{
    const StepCore::MetaProperty* property;
    if(sender() == _object1) property = _graphItem->graph()->metaObject()->property("object1");
    else property = _graphItem->graph()->metaObject()->property("object2");

    _updating = true;
    _graphItem->_worldModel->simulationPause();
    _graphItem->_worldModel->setProperty(_graphItem->graph(), property, text);
    _updating = false;
}

void GraphWidget::advance()
{
    if(!_updating) {
        _name->setText(_graphItem->graph()->name());
        if(_graphItem->graph()->object1() != _object1->currentText())
            _object1->setCurrentIndex(_object1->findData(_graphItem->graph()->object1(), Qt::DisplayRole));
        if(_graphItem->graph()->object2() != _object2->currentText())
            _object2->setCurrentIndex(_object2->findData(_graphItem->graph()->object2(), Qt::DisplayRole));
    }
}

GraphGraphicsItem::GraphGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Graph*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);

    _graphWidget = new GraphWidget(this);

    _boundingRect = QRectF(0, 0, 0, 0);
    _lastScale = 1;
    scale(1, -1);
    advance(1);
}

GraphGraphicsItem::~GraphGraphicsItem()
{
    delete _graphWidget;
}

inline StepCore::Graph* GraphGraphicsItem::graph() const
{
    return static_cast<StepCore::Graph*>(_item);
}

void GraphGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(Qt::black, 0));
    painter->setBrush(QBrush(Qt::white));
    QRectF rect = boundingRect();
    painter->drawRect(rect);
}

void GraphGraphicsItem::advance(int phase)
{
    if(phase == 0) return;

    double s = currentViewScale();
    if(s != _lastScale) {
        resetTransform();
        scale(1/s, -1/s);
        _lastScale = s;
    }
    
    QPointF r = vectorToPoint(graph()->position());
    StepCore::Vector2d vs = graph()->size();
    QSizeF vss(vs[0], vs[1]);

    /*
    QSizeF  size = _textItem->boundingRect().size()/s;
    size.setHeight(-size.height());

    if(size != _boundingRect.size()) {
        prepareGeometryChange();
        _boundingRect.setSize(size);
    }*/

    if(scene() && !scene()->views().isEmpty()) {
        QGraphicsView* activeView = scene()->views().first();

        // Reparent the widget if necessary.
        if(_graphWidget->parentWidget() != activeView->viewport()) {
           _graphWidget->setParent(activeView->viewport());
           _graphWidget->show();
        }

        QTransform itemTransform = deviceTransform(activeView->viewportTransform());
        QPoint viewportPos = itemTransform.map(QPointF(0, 0)).toPoint() + QPoint(1,1);
        _graphWidget->move(viewportPos);

        if(_graphWidget->size() != _boundingRect.size()) {
            _graphWidget->resize(vss.toSize());
        }

        _graphWidget->advance();
    }

    if(r != pos()) setPos(r);
    if(vss + QSizeF(2,2) != _boundingRect.size()) {
        prepareGeometryChange();
        _boundingRect.setSize(vss + QSizeF(2,2));
    }


    update(); // XXX: documentation says this is unnessesary, but it doesn't work without it
}

