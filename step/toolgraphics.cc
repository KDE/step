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

#include "ui_configure_graph.h"

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
#include <KPlotObject>
#include <KDialog>
#include <KAction>
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
        ++_noteItem->_updating;
        setPlainText("");
        _noteItem->worldDataChanged(false);
        --_noteItem->_updating;
    }
    QGraphicsTextItem::focusInEvent(event);
}

void NoteTextItem::focusOutEvent(QFocusEvent *event)
{
    if(_noteItem->note()->text().isEmpty()) {
        ++_noteItem->_updating;
        setPlainText(emptyNotice());
        _noteItem->worldDataChanged(false);
        --_noteItem->_updating;
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
    _updating = 0;
    connect(_textItem->document(), SIGNAL(contentsChanged()), this, SLOT(contentsChanged()));
    worldDataChanged(false);
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

void NoteGraphicsItem::viewScaleChanged()
{
    double s = currentViewScale();
    if(s != _lastScale) {
        _textItem->resetTransform();
        _textItem->scale(1/s, -1/s);
        _lastScale = s;
    }
    
    QSizeF  size = _textItem->boundingRect().size()/s;
    size.setHeight(-size.height());

    if(size != _boundingRect.size()) {
        prepareGeometryChange();
        _boundingRect.setSize(size);
    }
}

void NoteGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) {
        setPos(vectorToPoint(note()->position()));
        if(!_updating && _textItem->toPlainText() != note()->text()) {
            ++_updating;
            if(!_textItem->hasFocus() && note()->text().isEmpty()) {
                _textItem->setPlainText(_textItem->emptyNotice());
            } else {
                _textItem->setPlainText(note()->text());
            }
            --_updating;
        }
        viewScaleChanged();
        update();
    }
}

void NoteGraphicsItem::contentsChanged()
{
    if(!_updating) {
        ++_updating;
        _worldModel->simulationPause();
        _worldModel->setProperty(_item, _item->metaObject()->property("text"),
                                QVariant::fromValue( _textItem->toPlainText() ));
        --_updating;
    }
}

DataSourceWidget::DataSourceWidget(QWidget* parent)
    : QWidget(parent), _worldModel(0)
{
    _updating = 0;

    QHBoxLayout *layout = new QHBoxLayout(this);

    _object = new QComboBox(this);
    _object->setToolTip("Object name");
    layout->addWidget(_object, 1);

    _property = new QComboBox(this);
    _property->setToolTip("Property name");
    _property->setEnabled(false);
    layout->addWidget(_property, 1);

    _index = new QComboBox(this);
    _index->setToolTip("Vector index");
    _index->setMinimumContentsLength(1);
    _index->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    _index->hide();
    layout->addWidget(_index, 0);

    connect(_object, SIGNAL(activated(const QString&)),
            this, SLOT(objectSelected(const QString&)));
    connect(_property, SIGNAL(activated(const QString&)),
            this, SLOT(propertySelected(const QString&)));
    connect(_index, SIGNAL(activated(const QString&)),
            this, SLOT(indexSelected(const QString&)));
}

void DataSourceWidget::setDataSource(WorldModel* worldModel, const QString& object,
                                        const QString& property, int index)
{
    _worldModel = worldModel;
    if(!_worldModel) return;

    ++_updating;

    _object->clear();
    _object->addItem(_worldModel->world()->name());
    for(int i=0; i<_worldModel->itemCount(); ++i)
        _object->addItem(_worldModel->item(i)->name());
    for(int i=1; i<_worldModel->rowCount(); ++i)
        _object->addItem(_worldModel->index(i, 0).data(WorldModel::ObjectNameRole).toString());
    _object->setCurrentIndex( _object->findData(object, Qt::DisplayRole) );
    _property->setCurrentIndex( _property->findData(property, Qt::DisplayRole) );
    _index->setCurrentIndex( index );

    --_updating;
}

void DataSourceWidget::objectSelected(const QString& text)
{
    if(!_worldModel) return;
    kDebug() << "objectSelected" << endl;
    _property->clear();

    const StepCore::Object* obj = _worldModel->object(text);
    if(obj != 0) {
        _property->setEnabled(true);
        for(int i=0; i<obj->metaObject()->propertyCount(); ++i)
            _property->addItem(obj->metaObject()->property(i)->name());
    } else {
        _property->setEnabled(false);
        if(!_updating) emit dataSourceSelected(text, QString(), 0);
    }
}

void DataSourceWidget::propertySelected(const QString& text)
{
    if(!_worldModel) return;
    kDebug() << "propertySelected" << endl;
    const StepCore::Object* obj = _worldModel->object(_object->currentText());
    const StepCore::MetaProperty* pr = obj ? obj->metaObject()->property(text) : 0;

    if(pr != 0 && pr->userTypeId() == qMetaTypeId<StepCore::Vector2d>()) {
        _index->clear();
        _index->addItem("0");
        _index->addItem("1");
        _index->show();
        return;
    }

    _index->hide();
    emit dataSourceSelected(_object->currentText(), text, 0);
}

void DataSourceWidget::indexSelected(const QString& text)
{
    if(!_worldModel) return;
    kDebug() << "indexSelected" << endl;
    emit dataSourceSelected(_object->currentText(), _property->currentText(), text.toInt());
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
    _updating = 0;
    _lastPointTime = -HUGE_VALF;

    _graph = _graphItem->graph();
    _worldModel = _graphItem->_worldModel;

    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->setColumnStretch(0, 0);
    gridLayout->setColumnStretch(1, 5);
    gridLayout->setColumnStretch(2, 5);
    gridLayout->setColumnStretch(3, 0);

    gridLayout->setRowStretch(0, 0);
    gridLayout->setRowStretch(1, 1);
    gridLayout->setRowStretch(2, 0);
    gridLayout->setRowStretch(3, 0);

    _name = new QLabel(_graph->name(), this);
    _name->setAlignment(Qt::AlignHCenter);
    QFont font = _name->font(); font.setBold(true); _name->setFont(font);
    gridLayout->addWidget(_name, 0, 0, 1, -1);

    _plotWidget = new KPlotWidget(this);
    _plotWidget->setBackgroundColor(Qt::white);
    _plotWidget->setForegroundColor(Qt::black);
    //_plotWidget->setLeftPadding(0);
    _plotWidget->setTopPadding(2);
    _plotWidget->setRightPadding(3);

    _plotObject = new KPlotObject(Qt::black/*int(KPlotObject::Points|KPlotObject::Lines)*/);
    _plotObject->setShowPoints(true);
    _plotObject->setShowLines(true);
    _plotObject->setPointStyle(KPlotObject::Square);

    //_plotWidget->setAntialiasing(true);
    _plotWidget->addPlotObject(_plotObject);

    /*
    _plotObject->addPoint(0.5, 0.5);
    _plotObject->addPoint(0.7, 0.7);
    _plotObject->addPoint(0.9, 0.9);
    */

    gridLayout->addWidget(_plotWidget, 1, 0, 1, -1);

    QAbstractItemModel* model = new GraphFlatWorldModel(_worldModel, this);
    for(int i=0; i<2; ++i) {
        QLabel* label = new QLabel(i==0 ? "x:" : "y:", this);
        gridLayout->addWidget(label, 2+i, 0, 1, 1);

        _object[i] = new QComboBox(this);
        _object[i]->setToolTip("Object name");
        gridLayout->addWidget(_object[i], 2+i, 1, 1, 1);

        _property[i] = new QComboBox(this);
        _property[i]->setToolTip("Property name");
        _property[i]->setEnabled(false);
        gridLayout->addWidget(_property[i], 2+i, 2, 1, 1);

        _index[i] = new QComboBox(this);
        _index[i]->setToolTip("Vector index");
        _index[i]->setMinimumContentsLength(1);
        _index[i]->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
        _index[i]->hide();
        gridLayout->addWidget(_index[i], 2+i, 3, 1, 1);

        _object[i]->setModel(model);
        _object[i]->setCurrentIndex(-1);
        connect(_object[i], SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(objectSelected(const QString&)));
        connect(_property[i], SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(propertySelected(const QString&)));
        connect(_index[i], SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(indexSelected(const QString&)));
    }

    _clearAction = new KAction(i18n("Clear graph"), this);
    _configureAction = new KAction(i18n("Configure graph..."), this);
    connect(_configureAction, SIGNAL(triggered()), this, SLOT(configure()));
    _plotWidget->addAction(_clearAction);
    _plotWidget->addAction(_configureAction);
    _plotWidget->setContextMenuPolicy(Qt::ActionsContextMenu);

    _doclear = 0;
    worldDataChanged();
    _doclear = 1;
}

GraphWidget::~GraphWidget()
{
    _plotWidget->hide(); // BUG ?
    delete _plotWidget;
}

void GraphWidget::configure()
{
    KDialog* confDialog = new KDialog(this);
    confDialog->setCaption(i18n("Configure graph"));
    confDialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
    //DataSourceWidget* w = new DataSourceWidget(_worldModel, confDialog);
    //w->setDataSource(QString(), QString(), 0);
    Ui::WidgetConfigureGraph confUi;
    confUi.setupUi(confDialog->mainWidget());
    confUi.dataSourceX->setDataSource(_worldModel);
    confUi.dataSourceY->setDataSource(_worldModel);
    confDialog->exec();
    kDebug() << "exec finished" << endl;
    delete confDialog;
}

void GraphWidget::objectSelected(const QString& text)
{
    int n = (sender() == _object[0] ? 0 : 1);

    bool macro = false;

    if(!_updating) {
        ++_updating;
        macro = true;
        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Edit %1", _graph->name()));
        _worldModel->setProperty(_graph,
                _graph->metaObject()->property(n==0 ? "object1":"object2"), text);
        //_worldModel->endMacro();
        --_updating;
    }

    ++_updating;
    _property[n]->clear();
    const StepCore::Object* obj = (n==0 ? _graph->objectPtr1() : _graph->objectPtr2());
    if(obj) {
        for(int i=0; i<obj->metaObject()->propertyCount(); ++i) {
            const StepCore::MetaProperty* p = obj->metaObject()->property(i);
            if(p->userTypeId() == qMetaTypeId<double>() || p->userTypeId() == qMetaTypeId<StepCore::Vector2d>())
                _property[n]->addItem(p->name());
        }
        _property[n]->setEnabled(true);
    } else _property[n]->setEnabled(false);

    _property[n]->setCurrentIndex(-1);
    if(macro) {
        _worldModel->setProperty(_graph,
                _graph->metaObject()->property(n==0 ? "property1":"property2"), _property[n]->itemText(0));
        _property[n]->setCurrentIndex(0);
        _worldModel->endMacro();
    }
    --_updating;
}

void GraphWidget::propertySelected(const QString& text)
{
    int n = (sender() == _property[0] ? 0 : 1);

    if(!_updating) {
        ++_updating;
        _worldModel->beginMacro(i18n("Edit %1", _graph->name()));
        _worldModel->simulationPause();
        _worldModel->setProperty(_graph,
                _graph->metaObject()->property(n==0 ? "property1":"property2"), text);
        _worldModel->endMacro();
        --_updating;
    }

    ++_updating;
    _index[n]->clear();
    const StepCore::MetaProperty* p = (n==0 ? _graph->propertyPtr1() : _graph->propertyPtr2());
    if(p) {
        if(p->userTypeId() == qMetaTypeId<StepCore::Vector2d>()) {
            _index[n]->addItem("0");
            _index[n]->addItem("1");
            _index[n]->setCurrentIndex( n==0 ? _graph->index1() : _graph->index2() );
            if(_index[n]->isHidden()) _index[n]->show();
        } else {
            if(_index[n]->isVisible()) _index[n]->hide();
            indexSelected(QString());
        }
    } else {
        if(_index[n]->isVisible()) _index[n]->hide();
        indexSelected(QString());
    }
    --_updating;
}

void GraphWidget::indexSelected(const QString& text)
{
    int n = (sender() == _index[0] ? 0 : 1);

    if(!_updating) {
        ++_updating;
        _worldModel->beginMacro(i18n("Edit %1", _graph->name()));
        _worldModel->simulationPause();
        _worldModel->setProperty(_graph,
                _graph->metaObject()->property(n==0 ? "index1":"index2"), text);
        _worldModel->endMacro();
        --_updating;
    }

    if(_doclear) {
        _graph->clearPoints();
        _lastPointTime = -HUGE_VALF;
        recordPoint();
    } else {
        _lastPointTime = _worldModel->world()->time();
    }
    worldDataChanged();
}

void GraphWidget::worldDataChanged()
{
    if(!_updating) {
        ++_updating;
        _name->setText(_graph->name());

        if(_graph->object1() != _object[0]->currentText())
            _object[0]->setCurrentIndex(_object[0]->findData(_graph->object1(), Qt::DisplayRole));
        if(_graph->object2() != _object[1]->currentText())
            _object[1]->setCurrentIndex(_object[1]->findData(_graph->object2(), Qt::DisplayRole));

        if(_graph->property1() != _property[0]->currentText())
            _property[0]->setCurrentIndex(_property[0]->findData(_graph->property1(), Qt::DisplayRole));
        if(_graph->property1() != _property[1]->currentText())
            _property[1]->setCurrentIndex(_property[1]->findData(_graph->property2(), Qt::DisplayRole));

        if(_index[0]->isVisible()) _index[0]->setCurrentIndex(_graph->index1());
        if(_index[1]->isVisible()) _index[1]->setCurrentIndex(_graph->index2());

        --_updating;
    }

    if(_worldModel->world()->time() > _lastPointTime
                + 1.0/_worldModel->simulationFps() - 1e-2/_worldModel->simulationFps()) {
        recordPoint();
    }

    const StepCore::Vector2d& limX = _graph->limitsX();
    const StepCore::Vector2d& limY = _graph->limitsY();
    _plotWidget->setLimits(limX[0], limX[1], limY[0], limY[1]);

    _plotObject->clearPoints();
    for(int i=0; i<(int)_graph->points().size(); ++i) {
        StepCore::Vector2d p = _graph->points()[i];
        _plotObject->addPoint(p[0], p[1]);
    }
    _plotWidget->update();
}

void GraphWidget::recordPoint()
{
    bool ok;
    StepCore::Vector2d point = _graph->recordPoint(&ok);
    if(ok) {
        _lastPointTime = _worldModel->world()->time();
    } else {
        _lastPointTime = -HUGE_VALF;
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

void GraphGraphicsItem::viewScaleChanged()
{
    double s = currentViewScale();
    if(s != _lastScale) {
        resetTransform();
        scale(1/s, -1/s);
        _lastScale = s;
    }
    
    /*
    QSizeF  size = _textItem->boundingRect().size()/s;
    size.setHeight(-size.height());

    if(size != _boundingRect.size()) {
        prepareGeometryChange();
        _boundingRect.setSize(size);
    }*/

    setPos(vectorToPoint(graph()->position()));

    StepCore::Vector2d vs = graph()->size();
    QSizeF vss(vs[0]+2, vs[1]+2);

    if(vss != _boundingRect.size()) {
        prepareGeometryChange();
        _boundingRect.setSize(vss);
    }

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
            _graphWidget->resize(vss.toSize() - QSize(2,2));
        }
    }
}

void GraphGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) {
        viewScaleChanged();
    }
    _graphWidget->worldDataChanged();
}

