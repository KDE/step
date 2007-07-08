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
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

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
    layout->addWidget(_index, 0);

    connect(_object, SIGNAL(activated(const QString&)),
            this, SLOT(objectSelected(const QString&)));
    connect(_property, SIGNAL(activated(const QString&)),
            this, SLOT(propertySelected(const QString&)));

    connect(_object, SIGNAL(activated(const QString&)),
            this, SIGNAL(dataSourceChanged()));
    connect(_property, SIGNAL(activated(const QString&)),
            this, SIGNAL(dataSourceChanged()));
    connect(_index, SIGNAL(activated(const QString&)),
            this, SIGNAL(dataSourceChanged()));
}

void DataSourceWidget::setDataSource(WorldModel* worldModel, const QString& object,
                                        const QString& property, int index)
{
    _worldModel = worldModel;
    if(!_worldModel) return;

    _object->clear();
    _object->addItem(_worldModel->world()->name());
    for(int i=0; i<_worldModel->itemCount(); ++i)
        _object->addItem(_worldModel->item(i)->name());
    for(int i=1; i<_worldModel->rowCount(); ++i)
        _object->addItem(_worldModel->index(i, 0).data(WorldModel::ObjectNameRole).toString());

    _object->setCurrentIndex( _object->findData(object, Qt::DisplayRole) );
    objectSelected(object);

    _property->setCurrentIndex( _property->findData(property, Qt::DisplayRole) );
    propertySelected(property);

    _index->setCurrentIndex( index );
}

void DataSourceWidget::objectSelected(const QString& text)
{
    Q_ASSERT(_worldModel);

    _property->clear();

    const StepCore::Object* obj = _worldModel->object(text);
    if(obj != 0) {
        _property->setEnabled(true);
        for(int i=0; i<obj->metaObject()->propertyCount(); ++i) {
            const StepCore::MetaProperty* pr = obj->metaObject()->property(i);
            if(pr->userTypeId() == qMetaTypeId<double>() ||
                        pr->userTypeId() == qMetaTypeId<StepCore::Vector2d>()) {
                _property->addItem(pr->name());
            }
        }
        propertySelected(_property->currentText());
    } else {
        _property->setEnabled(false);
    }
}

void DataSourceWidget::propertySelected(const QString& text)
{
    Q_ASSERT(_worldModel);

    const StepCore::Object* obj = _worldModel->object(_object->currentText());
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
    _plotObject->setShowPoints(true);
    _plotObject->setShowLines(true);
    _plotObject->setPointStyle(KPlotObject::Square);

    //_plotWidget->setAntialiasing(true);
    _plotWidget->addPlotObject(_plotObject);

    _clearAction = new KAction(i18n("Clear graph"), this);
    _configureAction = new KAction(i18n("Configure graph..."), this);
    connect(_clearAction, SIGNAL(triggered()), this, SLOT(clearGraph()));
    connect(_configureAction, SIGNAL(triggered()), this, SLOT(configureGraph()));
    _plotWidget->addAction(_clearAction);
    _plotWidget->addAction(_configureAction);
    _plotWidget->setContextMenuPolicy(Qt::ActionsContextMenu);

    _boundingRect = QRectF(0, 0, 0, 0);
    _lastScale = 1;
    scale(1, -1);

    _lastPointTime = -HUGE_VAL;

    _confUi = 0;
    _confDialog = 0;
    _confChanged = false;
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

void GraphGraphicsItem::configureGraph()
{
    if(_worldModel->isSimulationActive())
        _worldModel->simulationStop();

    _confChanged = false;
    _confDialog = new KDialog(_plotWidget);
    
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

    _confDialog->exec();

    delete _confDialog; _confDialog = 0;
    delete _confUi; _confUi = 0;
}

void GraphGraphicsItem::confApply()
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

    _worldModel->endUpdate();
    _worldModel->endMacro();
}

void GraphGraphicsItem::confChanged()
{
    Q_ASSERT(_confUi && _confDialog);
    _confChanged = true;
    _confDialog->enableButtonApply(true);
}

void GraphGraphicsItem::clearGraph()
{
    _worldModel->simulationPause();
    _lastPointTime = -HUGE_VAL;
    _worldModel->setProperty(graph(), graph()->metaObject()->property("points"),
                               QVariant::fromValue(std::vector<StepCore::Vector2d>()) );
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
        }
        if(graph()->isValidY()) {
            labelY = i18n("%1.%2", graph()->objectY(), graph()->propertyY());
            if(graph()->indexY() >= 0) labelY.append(i18n("[%1]", graph()->indexY()));
        }
        _plotWidget->axis( KPlotWidget::BottomAxis )->setLabel(labelX);
        _plotWidget->axis( KPlotWidget::LeftAxis )->setLabel(labelY);

        if(!graph()->autoLimitsX() && !graph()->autoLimitsY()) adjustLimits();

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
    layout->addWidget(_labelSource, 1, 1, 1, 1);

    _configureAction = new KAction(i18n("Configure controller..."), this);
    connect(_configureAction, SIGNAL(triggered()), this, SLOT(configureGraph()));
    _widget->addAction(_configureAction);
    _widget->setContextMenuPolicy(Qt::ActionsContextMenu);

    _boundingRect = QRectF(0, 0, 0, 0);
    _lastScale = 1;
    scale(1, -1);

    _lastValue = 1;
    _changed = false;

    /*
    _confUi = 0;
    _confDialog = 0;
    _confChanged = false;
    */
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

void ControllerGraphicsItem::configureController()
{
#if 0
    if(_worldModel->isSimulationActive())
        _worldModel->simulationStop();

    _confChanged = false;
    _confDialog = new KDialog(_plotWidget);
    
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

    _confDialog->exec();

    delete _confDialog; _confDialog = 0;
    delete _confUi; _confUi = 0;
#endif
}

void ControllerGraphicsItem::confApply()
{
#if 0
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

    _worldModel->endUpdate();
    _worldModel->endMacro();
#endif
}

void ControllerGraphicsItem::confChanged()
{
    /*
    Q_ASSERT(_confUi && _confDialog);
    _confChanged = true;
    _confDialog->enableButtonApply(true);
    */
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
        }
        _labelSource->setText(source);

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

