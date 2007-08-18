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
#include "ui_configure_meter.h"
#include "ui_configure_controller.h"

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
#include <QLCDNumber>
#include <KPlotWidget>
#include <KPlotObject>
#include <KPlotPoint>
#include <KPlotAxis>
#include <KDialog>
#include <KAction>
#include <KLocale>

#include <float.h>

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
        if(!_updating && _textItem->toHtml() != note()->text()) {
            ++_updating;
            if(!_textItem->hasFocus() && note()->text().isEmpty()) {
                _textItem->setPlainText(_textItem->emptyNotice());
            } else {
                _textItem->setHtml(note()->text());
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
                                QVariant::fromValue( _textItem->toHtml() ));
        --_updating;
    }
}

DataSourceWidget::DataSourceWidget(QWidget* parent)
    : QWidget(parent), _worldModel(0)
{
    _skipReadOnly = false;
    
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    _object = new QComboBox(this);
    _object->setToolTip("Object name");
    _object->setMinimumContentsLength(10);
    layout->addWidget(_object, 1);

    _property = new QComboBox(this);
    _property->setToolTip("Property name");
    _property->setEnabled(false);
    _property->setMinimumContentsLength(10);
    layout->addWidget(_property, 1);

    _index = new QComboBox(this);
    _index->setToolTip("Vector index");
    _index->setMinimumContentsLength(1);
    _index->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    layout->addWidget(_index, 0);

    connect(_object, SIGNAL(activated(int)),
            this, SLOT(objectSelected(int)));
    connect(_property, SIGNAL(activated(int)),
            this, SLOT(propertySelected(int)));

    connect(_object, SIGNAL(activated(int)),
            this, SIGNAL(dataSourceChanged()));
    connect(_property, SIGNAL(activated(int)),
            this, SIGNAL(dataSourceChanged()));
    connect(_index, SIGNAL(activated(int)),
            this, SIGNAL(dataSourceChanged()));
}

void DataSourceWidget::addObjects(const QModelIndex& parent, const QString& indent)
{
    for(int i=0; i<_worldModel->rowCount(parent); ++i) {
        QModelIndex index = _worldModel->index(i, 0, parent);
        QString name = _worldModel->object(index)->name();
        if(name.isEmpty()) continue;
        _object->addItem(indent + name, name);
        addObjects(index, indent + ' ');
    }
}

void DataSourceWidget::setDataSource(WorldModel* worldModel, const QString& object,
                                        const QString& property, int index)
{
    _worldModel = worldModel;
    if(!_worldModel) return;

    _object->clear();

    addObjects(QModelIndex(), "");

    int objIndex = _object->findData(object);
    _object->setCurrentIndex( objIndex );
    objectSelected(objIndex);

    int propIndex = _property->findData(property);
    _property->setCurrentIndex( propIndex );
    propertySelected(propIndex);

    _index->setCurrentIndex( index );
}

void DataSourceWidget::objectSelected(int index)
{
    Q_ASSERT(_worldModel);

    _property->clear();

    QString text = _object->itemData(index).toString();
    const StepCore::Object* obj = _worldModel->object(text);
    if(obj != 0) {
        _property->setEnabled(true);
        for(int i=0; i<obj->metaObject()->propertyCount(); ++i) {
            const StepCore::MetaProperty* pr = obj->metaObject()->property(i);
            if(_skipReadOnly && !pr->isWritable()) continue;
            if(pr->userTypeId() == qMetaTypeId<double>() ||
                        pr->userTypeId() == qMetaTypeId<StepCore::Vector2d>()) {
                _property->addItem(pr->name(), pr->name());
            }
        }
        propertySelected(_property->currentIndex());
    } else {
        _property->setEnabled(false);
    }
}

void DataSourceWidget::propertySelected(int index)
{
    Q_ASSERT(_worldModel);

    QString text = _property->itemData(index).toString();
    const StepCore::Object* obj = _worldModel->object(
                            _object->itemData(_object->currentIndex()).toString());
    const StepCore::MetaProperty* pr = obj ? obj->metaObject()->property(text) : 0;

    _index->clear();
    if(pr != 0 && pr->userTypeId() == qMetaTypeId<StepCore::Vector2d>()) {
        _index->setEnabled(true);
        _index->addItem("0");
        _index->addItem("1");
    } else {
        _index->setEnabled(false);
    }
}

GraphGraphicsItem::GraphGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Graph*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);

    _plotWidget = new KPlotWidget();
    _plotWidget->setBackgroundColor(Qt::white);
    _plotWidget->setForegroundColor(Qt::black);
    //_plotWidget->setLeftPadding(0);
    //_plotWidget->setTopPadding(2);
    //_plotWidget->setRightPadding(3);

    _plotObject = new KPlotObject(Qt::black);
    _plotObject->setShowPoints(false);
    _plotObject->setShowLines(true);
    _plotObject->setPointStyle(KPlotObject::Square);

    _plotObject1 = new KPlotObject(Qt::red);
    _plotObject1->setShowPoints(true);
    _plotObject1->setShowLines(false);
    _plotObject1->setPointStyle(KPlotObject::Square);

    QList<KPlotObject*> plotObjects;
    plotObjects << _plotObject;
    plotObjects << _plotObject1;

    //_plotWidget->setAntialiasing(true);
    _plotWidget->addPlotObjects(plotObjects);


    _boundingRect = QRectF(0, 0, 0, 0);
    _lastScale = 1;
    scale(1, -1);

    _lastPointTime = -HUGE_VAL;
}

GraphGraphicsItem::~GraphGraphicsItem()
{
    _plotWidget->hide();
    delete _plotWidget;
}

inline StepCore::Graph* GraphGraphicsItem::graph() const
{
    return static_cast<StepCore::Graph*>(_item);
}

void GraphGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    // Do not need to fill the background since widget covers it all
    if(_isSelected) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(QBrush(Qt::white));
        painter->drawRect(_boundingRect);
    }
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
        update();
    }

    if(scene() && !scene()->views().isEmpty()) {
        QGraphicsView* activeView = scene()->views().first();

        // Reparent the widget if necessary.
        if(_plotWidget->parentWidget() != activeView->viewport()) {
           _plotWidget->setParent(activeView->viewport());
           _plotWidget->show();
        }

        QTransform itemTransform = deviceTransform(activeView->viewportTransform());
        QPoint viewportPos = itemTransform.map(QPointF(0, 0)).toPoint() + QPoint(1,1);

        if(_plotWidget->pos() != viewportPos)
            _plotWidget->move(viewportPos);

        if(_plotWidget->size() != _boundingRect.size())
            _plotWidget->resize(vss.toSize() - QSize(2,2));
    }
}

void GraphGraphicsItem::stateChanged()
{
    update();
}

void GraphGraphicsItem::adjustLimits()
{
    double minX =  HUGE_VAL, minY =  HUGE_VAL;
    double maxX = -HUGE_VAL, maxY = -HUGE_VAL;

    if(graph()->autoLimitsX() || graph()->autoLimitsY()) {
        for(int i=0; i<(int) graph()->points().size(); ++i) {
            StepCore::Vector2d p = graph()->points()[i];
            if(p[0] < minX) minX = p[0];
            if(p[0] > maxX) maxX = p[0];
            if(p[1] < minY) minY = p[1];
            if(p[1] > maxY) maxY = p[1];
        }
    }

    if(!graph()->autoLimitsX() || graph()->points().empty()) {
        minX = graph()->limitsX()[0];
        maxX = graph()->limitsX()[1];
    } else {
        double range = maxX - minX;
        if(range != 0) { minX -= 0.1*range; maxX += 0.1*range; }
        else { minX -= 0.5; maxX += 0.5; }
    }

    if(!graph()->autoLimitsY() || graph()->points().empty()) {
        minY = graph()->limitsY()[0];
        maxY = graph()->limitsY()[1];
    } else {
        double range = maxY - minY;
        if(range != 0) { minY -= 0.1*range; maxY += 0.1*range; }
        else { minY -= 0.5; maxY += 0.5; }
    }

    _plotWidget->setLimits(minX, maxX, minY, maxY);
}

void GraphGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) {
        viewScaleChanged();

        // Labels
        QString labelX, labelY;
        if(graph()->isValidX()) {
            labelX = i18n("%1.%2", graph()->objectX(), graph()->propertyX());
            if(graph()->indexX() >= 0) labelX.append(i18n("[%1]", graph()->indexX()));
            QString units = graph()->unitsX();
            if(!units.isEmpty()) labelX.append(" [").append(units).append("]");
        } else {
            labelX = i18n("[not configured]");
        }
        if(graph()->isValidY()) {
            labelY = i18n("%1.%2", graph()->objectY(), graph()->propertyY());
            if(graph()->indexY() >= 0) labelY.append(i18n("[%1]", graph()->indexY()));
            QString units = graph()->unitsY();
            if(!units.isEmpty()) labelY.append(" [").append(units).append("]");
        } else {
            labelY = i18n("[not configured]");
        }
        _plotWidget->axis( KPlotWidget::BottomAxis )->setLabel(labelX);
        _plotWidget->axis( KPlotWidget::LeftAxis )->setLabel(labelY);

        if(!graph()->autoLimitsX() && !graph()->autoLimitsY()) adjustLimits();

        _plotObject->setShowPoints(graph()->showPoints());
        _plotObject->setShowLines(graph()->showLines());

        /*
        // Points
        _plotObject->clearPoints();
        for(int i=0; i<(int) graph()->points().size(); ++i) {
            StepCore::Vector2d p = graph()->points()[i];
            _plotObject->addPoint(p[0], p[1]);
        }

        adjustLimits();
        _plotWidget->update();
        */

    }
    
    if(_worldModel->isSimulationActive()) {
        if(_worldModel->world()->time() > _lastPointTime
                    + 1.0/_worldModel->simulationFps() - 1e-2/_worldModel->simulationFps()) {
            StepCore::Vector2d point = graph()->recordPoint();
            _lastPointTime = _worldModel->world()->time();
        }
    }

    int po_count, p_count;
    do {
        const QList<KPlotPoint*> points = _plotObject->points();
        po_count = points.count(); p_count = graph()->points().size();
        int count = qMin(po_count, p_count);
        for(int p=0; p < count; ++p)
            points[p]->setPosition(vectorToPoint(graph()->points()[p]));
    } while(0);

    if(po_count < p_count) {
        for(; po_count < p_count; ++po_count)
            _plotObject->addPoint(vectorToPoint(graph()->points()[po_count]));
    } else {
        for(--po_count; po_count >= p_count; --po_count)
            _plotObject->removePoint(po_count);
    }

    if(p_count > 0) {
        if(_plotObject1->points().isEmpty()) {
            _plotObject1->addPoint(0,0);
        }
        _plotObject1->points()[0]->setPosition(vectorToPoint(graph()->points()[p_count-1]));
    } else {
        _plotObject1->clearPoints();
    }

    if(graph()->autoLimitsX() || graph()->autoLimitsY()) adjustLimits();
    _plotWidget->update();

#if 0
//#error Do setProperty here and remove DynamicOnly from points
        if(ok) {
            _plotObject->addPoint(point[0], point[1]);
            if(graph()->autoLimitsX() || graph()->autoLimitsY()) 
                adjustLimits();
            _plotWidget->update();
        }
        _lastPointTime = _worldModel->world()->time();
        worldDataChanged(false);
        //_worldModel->setProperty(graph(), graph()->metaObject()->property("name"), QString("test"));
    }
#endif
}

void GraphMenuHandler::populateMenu(QMenu* menu)
{
    _confUi = 0;
    _confDialog = 0;
    _confChanged = false;

    menu->addAction(KIcon("edit-clear"), i18n("Clear graph"), this, SLOT(clearGraph()));
    menu->addAction(KIcon("configure"), i18n("Configure graph..."), this, SLOT(configureGraph()));
    menu->addSeparator();
    ItemMenuHandler::populateMenu(menu);
}

inline StepCore::Graph* GraphMenuHandler::graph() const
{
    return static_cast<StepCore::Graph*>(_object);
}

void GraphMenuHandler::configureGraph()
{
    if(_worldModel->isSimulationActive())
        _worldModel->simulationStop();

    _confChanged = false;
    _confDialog = new KDialog(); // XXX: parent?
    
    _confDialog->setCaption(i18n("Configure graph"));
    _confDialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);

    _confUi = new Ui::WidgetConfigureGraph;
    _confUi->setupUi(_confDialog->mainWidget());

    _confUi->dataSourceX->setDataSource(_worldModel, graph()->objectX(),
                                    graph()->propertyX(), graph()->indexX());
    _confUi->dataSourceY->setDataSource(_worldModel, graph()->objectY(),
                                    graph()->propertyY(), graph()->indexY());

    _confUi->checkBoxAutoX->setChecked(graph()->autoLimitsX());
    _confUi->checkBoxAutoY->setChecked(graph()->autoLimitsY());

    _confUi->lineEditMinX->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditMinX));
    _confUi->lineEditMaxX->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditMaxX));
    _confUi->lineEditMinY->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditMinY));
    _confUi->lineEditMaxY->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditMaxY));

    _confUi->lineEditMinX->setText(QString::number(graph()->limitsX()[0]));
    _confUi->lineEditMaxX->setText(QString::number(graph()->limitsX()[1]));
    _confUi->lineEditMinY->setText(QString::number(graph()->limitsY()[0]));
    _confUi->lineEditMaxY->setText(QString::number(graph()->limitsY()[1]));

    _confUi->checkBoxShowLines->setChecked(graph()->showLines());
    _confUi->checkBoxShowPoints->setChecked(graph()->showPoints());

    _confDialog->enableButtonApply(false);

    connect(_confDialog, SIGNAL(applyClicked()), this, SLOT(confApply()));
    connect(_confDialog, SIGNAL(okClicked()), this, SLOT(confApply()));

    connect(_confUi->dataSourceX, SIGNAL(dataSourceChanged()), this, SLOT(confChanged()));
    connect(_confUi->dataSourceY, SIGNAL(dataSourceChanged()), this, SLOT(confChanged()));
    connect(_confUi->checkBoxAutoX, SIGNAL(stateChanged(int)), this, SLOT(confChanged()));
    connect(_confUi->checkBoxAutoY, SIGNAL(stateChanged(int)), this, SLOT(confChanged()));
    connect(_confUi->lineEditMinX, SIGNAL(textEdited(const QString&)), this, SLOT(confChanged()));
    connect(_confUi->lineEditMaxX, SIGNAL(textEdited(const QString&)), this, SLOT(confChanged()));
    connect(_confUi->lineEditMinY, SIGNAL(textEdited(const QString&)), this, SLOT(confChanged()));
    connect(_confUi->lineEditMaxY, SIGNAL(textEdited(const QString&)), this, SLOT(confChanged()));
    connect(_confUi->checkBoxShowLines, SIGNAL(stateChanged(int)), this, SLOT(confChanged()));
    connect(_confUi->checkBoxShowPoints, SIGNAL(stateChanged(int)), this, SLOT(confChanged()));

    _confDialog->exec();

    delete _confDialog; _confDialog = 0;
    delete _confUi; _confUi = 0;
}

void GraphMenuHandler::confApply()
{
    Q_ASSERT(_confUi && _confDialog);

    // XXX: check for actual change ?
    if(!_confChanged) return;
    _worldModel->beginMacro(i18n("Edit %1", graph()->name()));
    _worldModel->beginUpdate();

    _worldModel->setProperty(graph(), graph()->metaObject()->property("objectX"),
                                _confUi->dataSourceX->dataObject());
    _worldModel->setProperty(graph(), graph()->metaObject()->property("propertyX"),
                                _confUi->dataSourceX->dataProperty());
    _worldModel->setProperty(graph(), graph()->metaObject()->property("indexX"),
                                _confUi->dataSourceX->dataIndex());

    _worldModel->setProperty(graph(), graph()->metaObject()->property("objectY"),
                                _confUi->dataSourceY->dataObject());
    _worldModel->setProperty(graph(), graph()->metaObject()->property("propertyY"),
                                _confUi->dataSourceY->dataProperty());
    _worldModel->setProperty(graph(), graph()->metaObject()->property("indexY"),
                                _confUi->dataSourceY->dataIndex());

    _worldModel->setProperty(graph(), graph()->metaObject()->property("autoLimitsX"),
                                _confUi->checkBoxAutoX->isChecked());
    _worldModel->setProperty(graph(), graph()->metaObject()->property("autoLimitsY"),
                                _confUi->checkBoxAutoY->isChecked());

    StepCore::Vector2d limitsX(_confUi->lineEditMinX->text().toDouble(),
                               _confUi->lineEditMaxX->text().toDouble());
    StepCore::Vector2d limitsY(_confUi->lineEditMinY->text().toDouble(),
                               _confUi->lineEditMaxY->text().toDouble());

    _worldModel->setProperty(graph(), graph()->metaObject()->property("limitsX"),
                                        QVariant::fromValue(limitsX));
    _worldModel->setProperty(graph(), graph()->metaObject()->property("limitsY"),
                                        QVariant::fromValue(limitsY));

    _worldModel->setProperty(graph(), graph()->metaObject()->property("showLines"),
                                _confUi->checkBoxShowLines->isChecked());
    _worldModel->setProperty(graph(), graph()->metaObject()->property("showPoints"),
                                _confUi->checkBoxShowPoints->isChecked());

    _worldModel->endUpdate();
    _worldModel->endMacro();
}

void GraphMenuHandler::confChanged()
{
    Q_ASSERT(_confUi && _confDialog);
    _confChanged = true;
    _confDialog->enableButtonApply(true);
}

void GraphMenuHandler::clearGraph()
{
    _worldModel->simulationPause();
    //_lastPointTime = -HUGE_VAL; // XXX
    _worldModel->setProperty(graph(), graph()->metaObject()->property("points"),
                               QVariant::fromValue(std::vector<StepCore::Vector2d>()) );
}

////////////////////////////////////////////////////
MeterGraphicsItem::MeterGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Meter*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);

    _widget = new QFrame();
    _widget->setFrameShape(QFrame::Box);

    QGridLayout* layout = new QGridLayout(_widget);
    layout->setContentsMargins(0,0,2,0);
    layout->setSpacing(0);

    _lcdNumber = new QLCDNumber(_widget);
    _lcdNumber->setFrameShape(QFrame::NoFrame);
    _lcdNumber->setSegmentStyle(QLCDNumber::Flat);
    _lcdNumber->display(0);

    _labelUnits = new QLabel(_widget);
    _labelUnits->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    layout->addWidget(_lcdNumber, 0, 0, 1, 1);
    layout->addWidget(_labelUnits, 0, 1, 1, 1);

    _boundingRect = QRectF(0, 0, 0, 0);
    _lastScale = 1;
    _lastValue = 0;
    scale(1, -1);
}

MeterGraphicsItem::~MeterGraphicsItem()
{
    _widget->hide();
    delete _widget;
}

inline StepCore::Meter* MeterGraphicsItem::meter() const
{
    return static_cast<StepCore::Meter*>(_item);
}

void MeterGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    if(_isSelected) painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
    else painter->setPen(QPen(Qt::white));
    painter->setBrush(QBrush(Qt::white));
    painter->drawRect(_boundingRect);
}

void MeterGraphicsItem::viewScaleChanged()
{
    double s = currentViewScale();
    if(s != _lastScale) {
        resetTransform();
        scale(1/s, -1/s);
        _lastScale = s;
    }
    
    setPos(vectorToPoint(meter()->position()));

    StepCore::Vector2d vs = meter()->size();
    QSizeF vss(vs[0]+2, vs[1]+2);

    if(vss != _boundingRect.size()) {
        prepareGeometryChange();
        _boundingRect.setSize(vss);
        update();
    }

    if(scene() && !scene()->views().isEmpty()) {
        QGraphicsView* activeView = scene()->views().first();

        // Reparent the widget if necessary.
        if(_widget->parentWidget() != activeView->viewport()) {
           _widget->setParent(activeView->viewport());
           _widget->show();
        }

        QTransform itemTransform = deviceTransform(activeView->viewportTransform());
        QPoint viewportPos = itemTransform.map(QPointF(0, 0)).toPoint() + QPoint(1,1);

        if(_widget->pos() != viewportPos)
            _widget->move(viewportPos);

        if(_widget->size() != _boundingRect.size())
            _widget->resize(vss.toSize() - QSize(2,2));
    }
}

void MeterGraphicsItem::stateChanged()
{
    update();
}

void MeterGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) {
        viewScaleChanged();

        if(meter()->digits() != _lcdNumber->numDigits())
            _lcdNumber->setNumDigits(meter()->digits());

        QString units = meter()->units();
        if(!units.isEmpty()) {
             if(units != _labelUnits->text())
                 _labelUnits->setText(units);
        }
    }

    double value = meter()->value();
    _lcdNumber->display(value);

    //_lastValue = value;
}

void MeterMenuHandler::populateMenu(QMenu* menu)
{
    _confUi = 0;
    _confDialog = 0;
    _confChanged = false;

    menu->addAction(KIcon("configure"), i18n("Configure meter..."), this, SLOT(configureMeter()));
    menu->addSeparator();
    ItemMenuHandler::populateMenu(menu);
}

inline StepCore::Meter* MeterMenuHandler::meter() const
{
    return static_cast<StepCore::Meter*>(_object);
}

void MeterMenuHandler::configureMeter()
{
    if(_worldModel->isSimulationActive())
        _worldModel->simulationStop();

    _confChanged = false;
    _confDialog = new KDialog(); // XXX
    
    _confDialog->setCaption(i18n("Configure meter"));
    _confDialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);

    _confUi = new Ui::WidgetConfigureMeter;
    _confUi->setupUi(_confDialog->mainWidget());

    _confUi->dataSource->setDataSource(_worldModel, meter()->object(),
                                    meter()->property(), meter()->index());

    _confUi->lineEditDigits->setValidator(
                new QIntValidator(0, 100, _confUi->lineEditDigits));
    _confUi->lineEditDigits->setText(QString::number(meter()->digits()));

    connect(_confDialog, SIGNAL(applyClicked()), this, SLOT(confApply()));
    connect(_confDialog, SIGNAL(okClicked()), this, SLOT(confApply()));

    connect(_confUi->dataSource, SIGNAL(dataSourceChanged()), this, SLOT(confChanged()));
    connect(_confUi->lineEditDigits, SIGNAL(textEdited(const QString&)), this, SLOT(confChanged()));

    _confDialog->exec();

    delete _confDialog; _confDialog = 0;
    delete _confUi; _confUi = 0;
}

void MeterMenuHandler::confApply()
{
    Q_ASSERT(_confUi && _confDialog);

    // XXX: check for actual change ?
    if(!_confChanged) return;
    _worldModel->beginMacro(i18n("Edit %1", meter()->name()));
    _worldModel->beginUpdate();

    _worldModel->setProperty(meter(), meter()->metaObject()->property("object"),
                                _confUi->dataSource->dataObject());
    _worldModel->setProperty(meter(), meter()->metaObject()->property("property"),
                                _confUi->dataSource->dataProperty());
    _worldModel->setProperty(meter(), meter()->metaObject()->property("index"),
                                _confUi->dataSource->dataIndex());

    _worldModel->setProperty(meter(), meter()->metaObject()->property("digits"),
                                _confUi->lineEditDigits->text().toInt());

    _worldModel->endUpdate();
    _worldModel->endMacro();
}

void MeterMenuHandler::confChanged()
{
    Q_ASSERT(_confUi && _confDialog);
    _confChanged = true;
    _confDialog->enableButtonApply(true);
}

////////////////////////////////////////////////////
ControllerGraphicsItem::ControllerGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Controller*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);

    _widget = new QWidget();
    QGridLayout* layout = new QGridLayout(_widget);

    _labelMin = new QLabel(_widget); _labelMin->setAlignment(Qt::AlignRight);
    _labelMax = new QLabel(_widget); _labelMax->setAlignment(Qt::AlignLeft);
    _labelSource = new QLabel(_widget); _labelSource->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    _slider = new QSlider(Qt::Horizontal, _widget);
    _slider->setRange(SLIDER_MIN, SLIDER_MAX);
    connect(_slider, SIGNAL(sliderMoved(int)), this, SLOT(sliderChanged(int)));
    connect(_slider, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));

    layout->addWidget(_labelMin, 0, 0, 1, 1);
    layout->addWidget(_slider, 0, 1, 1, 1);
    layout->addWidget(_labelMax, 0, 2, 1, 1);
    layout->addWidget(_labelSource, 1, 0, 1, 3);

    _incAction = new KAction(i18n("Increase value"), _widget);
    _decAction = new KAction(i18n("Decrease value"), _widget);

    connect(_incAction, SIGNAL(triggered(bool)), this, SLOT(incTriggered()));
    connect(_decAction, SIGNAL(triggered(bool)), this, SLOT(decTriggered()));

    _widget->addAction(_incAction);
    _widget->addAction(_decAction);
    //_widget->addAction(_configureAction);
    //_widget->setContextMenuPolicy(Qt::ActionsContextMenu);

    _boundingRect = QRectF(0, 0, 0, 0);
    _lastScale = 1;
    scale(1, -1);

    _lastValue = 1;
    _changed = false;
}

ControllerGraphicsItem::~ControllerGraphicsItem()
{
    _widget->hide();
    delete _widget;
}

inline StepCore::Controller* ControllerGraphicsItem::controller() const
{
    return static_cast<StepCore::Controller*>(_item);
}

void ControllerGraphicsItem::decTriggered()
{
    _worldModel->simulationPause();
    _worldModel->setProperty(controller(), controller()->metaObject()->property("value"),
                                controller()->value() - controller()->increment());
}

void ControllerGraphicsItem::incTriggered()
{
    _worldModel->simulationPause();
    _worldModel->setProperty(controller(), controller()->metaObject()->property("value"),
                                controller()->value() + controller()->increment());
}

void ControllerGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    if(_isSelected) painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
    else painter->setPen(QPen(Qt::white));
    painter->setBrush(QBrush(Qt::white));
    painter->drawRect(_boundingRect);
}

void ControllerGraphicsItem::viewScaleChanged()
{
    double s = currentViewScale();
    if(s != _lastScale) {
        resetTransform();
        scale(1/s, -1/s);
        _lastScale = s;
    }
    
    setPos(vectorToPoint(controller()->position()));

    StepCore::Vector2d vs = controller()->size();
    QSizeF vss(vs[0]+2, vs[1]+2);

    if(vss != _boundingRect.size()) {
        prepareGeometryChange();
        _boundingRect.setSize(vss);
        update();
    }

    if(scene() && !scene()->views().isEmpty()) {
        QGraphicsView* activeView = scene()->views().first();

        // Reparent the widget if necessary.
        if(_widget->parentWidget() != activeView->viewport()) {
           _widget->setParent(activeView->viewport());
           _widget->show();
        }

        QTransform itemTransform = deviceTransform(activeView->viewportTransform());
        QPoint viewportPos = itemTransform.map(QPointF(0, 0)).toPoint() + QPoint(1,1);

        if(_widget->pos() != viewportPos)
            _widget->move(viewportPos);

        if(_widget->size() != _boundingRect.size())
            _widget->resize(vss.toSize() - QSize(2,2));
    }
}

void ControllerGraphicsItem::stateChanged()
{
    update();
}

void ControllerGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) {
        viewScaleChanged();

        // Labels
        _labelMin->setText(QString::number(controller()->limits()[0]));
        _labelMax->setText(QString::number(controller()->limits()[1]));

        QString source;
        if(controller()->isValid()) {
            source = i18n("%1.%2", controller()->object(), controller()->property());
            if(controller()->index() >= 0) source.append(i18n("[%1]", controller()->index()));
            QString units = controller()->units();
            if(!units.isEmpty()) source.append(" [").append(units).append("]");
        } else {
            source = i18n("[not configured]");
        }
        _labelSource->setText(source);

        if(_incAction->isEnabled() != controller()->isValid()) {
            _incAction->setEnabled(controller()->isValid());
            _decAction->setEnabled(controller()->isValid());
        }

        if(_incShortcut != controller()->increaseShortcut()) {
            _incShortcut = controller()->increaseShortcut();
            _incAction->setShortcut(KShortcut(_incShortcut));
        }

        if(_decShortcut != controller()->decreaseShortcut()) {
            _decShortcut = controller()->decreaseShortcut();
            _decAction->setShortcut(KShortcut(_decShortcut));
        }

        //if(!graph()->autoLimitsX() && !graph()->autoLimitsY()) adjustLimits();

        /*
        // Points
        _plotObject->clearPoints();
        for(int i=0; i<(int) graph()->points().size(); ++i) {
            StepCore::Vector2d p = graph()->points()[i];
            _plotObject->addPoint(p[0], p[1]);
        }

        adjustLimits();
        _plotWidget->update();
        */
    }

    double value = round((controller()->value() - controller()->limits()[0]) *
            (SLIDER_MAX - SLIDER_MIN) / (controller()->limits()[1] - controller()->limits()[0]) + SLIDER_MIN);

    if(value <= SLIDER_MIN && _lastValue > SLIDER_MIN) {
        QPalette palette; palette.setColor(_labelMin->foregroundRole(), Qt::red);
        _labelMin->setPalette(palette);
    } else if(value > SLIDER_MIN && _lastValue <= SLIDER_MIN) {
        QPalette palette; _labelMin->setPalette(palette);
    }

    if(value >= SLIDER_MAX-1 && _lastValue < SLIDER_MAX-1) {
        QPalette palette; palette.setColor(_labelMax->foregroundRole(), Qt::red);
        _labelMax->setPalette(palette);
    } else if(value < SLIDER_MAX-1 && _lastValue >= SLIDER_MAX-1) {
        QPalette palette; _labelMax->setPalette(palette);
    }

    _lastValue = value;

    if(value < SLIDER_MIN) value = SLIDER_MIN;
    else if(value > SLIDER_MAX-1) value = SLIDER_MAX-1;

    _slider->setValue(int(value));

#if 0
    
    if(_worldModel->isSimulationActive()) {
        if(_worldModel->world()->time() > _lastPointTime
                    + 1.0/_worldModel->simulationFps() - 1e-2/_worldModel->simulationFps()) {
            StepCore::Vector2d point = graph()->recordPoint();
            _lastPointTime = _worldModel->world()->time();
        }
    }

    int po_count, p_count;
    do {
        const QList<KPlotPoint*> points = _plotObject->points();
        po_count = points.count(); p_count = graph()->points().size();
        int count = qMin(po_count, p_count);
        for(int p=0; p < count; ++p)
            points[p]->setPosition(vectorToPoint(graph()->points()[p]));
    } while(0);

    if(po_count < p_count) {
        for(; po_count < p_count; ++po_count)
            _plotObject->addPoint(vectorToPoint(graph()->points()[po_count]));
    } else {
        for(--po_count; po_count >= p_count; --po_count)
            _plotObject->removePoint(po_count);
    }

    if(graph()->autoLimitsX() || graph()->autoLimitsY()) adjustLimits();
    _plotWidget->update();
#endif
#if 0
//#error Do setProperty here and remove DynamicOnly from points
        if(ok) {
            _plotObject->addPoint(point[0], point[1]);
            if(graph()->autoLimitsX() || graph()->autoLimitsY()) 
                adjustLimits();
            _plotWidget->update();
        }
        _lastPointTime = _worldModel->world()->time();
        worldDataChanged(false);
        //_worldModel->setProperty(graph(), graph()->metaObject()->property("name"), QString("test"));
    }
#endif
}

void ControllerGraphicsItem::sliderChanged(int value)
{
    Q_ASSERT(value == _slider->sliderPosition());
    if(!controller()->isValid()) return;
    //if(!_worldModel->isSimulationActive()) {
        _worldModel->simulationPause();
        if(!_changed) {
            _worldModel->beginMacro(i18n("Edit %1", controller()->object()));
            _changed = true;
        }
        double v = controller()->limits()[0] + (value - SLIDER_MIN) *
                (controller()->limits()[1] - controller()->limits()[0]) / (SLIDER_MAX - SLIDER_MIN);
        _worldModel->setProperty(controller(), controller()->metaObject()->property("value"), v);
    //}
}

void ControllerGraphicsItem::sliderReleased()
{
    if(_changed) {
        _worldModel->endMacro();
        _changed = false;
    }
}

void ControllerMenuHandler::populateMenu(QMenu* menu)
{
    _confUi = 0;
    _confDialog = 0;
    _confChanged = false;

    menu->addAction(KIcon("arrow-up"), i18n("Increase value"), this, SLOT(incTriggered()));
    menu->addAction(KIcon("arrow-down"), i18n("Decrease value"), this, SLOT(decTriggered()));
    menu->addSeparator();
    menu->addAction(KIcon("configure"), i18n("Configure controller..."), this, SLOT(configureController()));
    menu->addSeparator();
    ItemMenuHandler::populateMenu(menu);
}

inline StepCore::Controller* ControllerMenuHandler::controller() const
{
    return static_cast<StepCore::Controller*>(_object);
}

void ControllerMenuHandler::configureController()
{
    if(_worldModel->isSimulationActive())
        _worldModel->simulationStop();

    _confChanged = false;
    _confDialog = new KDialog(); // XXX
    
    _confDialog->setCaption(i18n("Configure controller"));
    _confDialog->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);

    _confUi = new Ui::WidgetConfigureController;
    _confUi->setupUi(_confDialog->mainWidget());

    _confUi->dataSource->setSkipReadOnly(true);
    _confUi->dataSource->setDataSource(_worldModel, controller()->object(),
                                    controller()->property(), controller()->index());

    _confUi->lineEditMin->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditMin));
    _confUi->lineEditMax->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditMax));

    _confUi->lineEditMin->setText(QString::number(controller()->limits()[0]));
    _confUi->lineEditMax->setText(QString::number(controller()->limits()[1]));

    _confUi->keyIncrease->setModifierlessAllowed(true);
    _confUi->keyDecrease->setModifierlessAllowed(true);

    //_confUi->keyIncrease->setKeySequence(_incAction->shortcut().primary());
    //_confUi->keyDecrease->setKeySequence(_decAction->shortcut().primary());
    _confUi->keyIncrease->setKeySequence(KShortcut(controller()->increaseShortcut()).primary());
    _confUi->keyDecrease->setKeySequence(KShortcut(controller()->decreaseShortcut()).primary());

    _confUi->lineEditIncrement->setValidator(
                new QDoubleValidator(-HUGE_VAL, HUGE_VAL, DBL_DIG, _confUi->lineEditIncrement));
    _confUi->lineEditIncrement->setText(QString::number(controller()->increment()));

    _confDialog->enableButtonApply(false);

    connect(_confDialog, SIGNAL(applyClicked()), this, SLOT(confApply()));
    connect(_confDialog, SIGNAL(okClicked()), this, SLOT(confApply()));

    connect(_confUi->dataSource, SIGNAL(dataSourceChanged()), this, SLOT(confChanged()));
    connect(_confUi->lineEditMin, SIGNAL(textEdited(const QString&)), this, SLOT(confChanged()));
    connect(_confUi->lineEditMax, SIGNAL(textEdited(const QString&)), this, SLOT(confChanged()));
    connect(_confUi->keyIncrease, SIGNAL(keySequenceChanged(const QKeySequence&)), this, SLOT(confChanged()));
    connect(_confUi->keyDecrease, SIGNAL(keySequenceChanged(const QKeySequence&)), this, SLOT(confChanged()));
    connect(_confUi->lineEditIncrement, SIGNAL(textEdited(const QString&)), this, SLOT(confChanged()));

    _confDialog->exec();

    delete _confDialog; _confDialog = 0;
    delete _confUi; _confUi = 0;
}

void ControllerMenuHandler::confApply()
{
    Q_ASSERT(_confUi && _confDialog);

    // XXX: check for actual change ?
    if(!_confChanged) return;
    _worldModel->beginMacro(i18n("Edit %1", controller()->name()));
    _worldModel->beginUpdate();

    _worldModel->setProperty(controller(), controller()->metaObject()->property("object"),
                                _confUi->dataSource->dataObject());
    _worldModel->setProperty(controller(), controller()->metaObject()->property("property"),
                                _confUi->dataSource->dataProperty());
    _worldModel->setProperty(controller(), controller()->metaObject()->property("index"),
                                _confUi->dataSource->dataIndex());

    StepCore::Vector2d limits(_confUi->lineEditMin->text().toDouble(),
                              _confUi->lineEditMax->text().toDouble());

    _worldModel->setProperty(controller(), controller()->metaObject()->property("limits"),
                                        QVariant::fromValue(limits));

    _worldModel->setProperty(controller(), controller()->metaObject()->property("increaseShortcut"),
                                 QVariant::fromValue(_confUi->keyIncrease->keySequence().toString()));

    _worldModel->setProperty(controller(), controller()->metaObject()->property("decreaseShortcut"),
                                 QVariant::fromValue(_confUi->keyDecrease->keySequence().toString()));

    _worldModel->setProperty(controller(), controller()->metaObject()->property("increment"),
                                 QVariant::fromValue(_confUi->lineEditIncrement->text().toDouble()));

    _worldModel->endUpdate();
    _worldModel->endMacro();
}

void ControllerMenuHandler::confChanged()
{
    Q_ASSERT(_confUi && _confDialog);
    _confChanged = true;
    _confDialog->enableButtonApply(true);
}

void ControllerMenuHandler::decTriggered()
{
    _worldModel->simulationPause();
    _worldModel->setProperty(controller(), controller()->metaObject()->property("value"),
                                controller()->value() - controller()->increment());
}

void ControllerMenuHandler::incTriggered()
{
    _worldModel->simulationPause();
    _worldModel->setProperty(controller(), controller()->metaObject()->property("value"),
                                controller()->value() + controller()->increment());
}

