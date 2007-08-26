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

#include "softbodygraphics.h"
#include "softbodygraphics.moc"

#include <stepcore/softbody.h>

#include "ui_create_softbody_items.h"

#include "worldmodel.h"
#include "worldfactory.h"
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QEvent>
#include <QPainter>
#include <QValidator>
#include <KDialog>
#include <KLocale>

#include <float.h>

void SoftBodyMenuHandler::populateMenu(QMenu* menu)
{
    _createSoftBodyItemsUi = 0;
    _createSoftBodyItemsDialog = 0;
    //_confChanged = false;

    // XXX: better icon
    menu->addAction(KIcon("step_object_GasParticle"), i18n("Create items..."), this, SLOT(createSoftBodyItems()));
    //menu->addAction(KIcon("edit-clear"), i18n("Clear gas"), this, SLOT(clearGas()));
    menu->addSeparator();
    ItemMenuHandler::populateMenu(menu);
}

inline StepCore::SoftBody* SoftBodyMenuHandler::softBody() const
{
    return static_cast<StepCore::SoftBody*>(_object);
}

void SoftBodyMenuHandler::clearSoftBody()
{
//    _worldModel->simulationPause();
}

void SoftBodyMenuHandler::createSoftBodyItems()
{
    if(_worldModel->isSimulationActive())
        _worldModel->simulationStop();

    _createSoftBodyItemsDialog = new KDialog(); // XXX: parent?
    
    _createSoftBodyItemsDialog->setCaption(i18n("Create soft body items"));
    _createSoftBodyItemsDialog->setButtons(KDialog::Ok | KDialog::Cancel);

    _createSoftBodyItemsUi = new Ui::WidgetCreateSoftBodyItems;
    _createSoftBodyItemsUi->setupUi(_createSoftBodyItemsDialog->mainWidget());

    _createSoftBodyItemsUi->lineEditPosition->setValidator(
                new QRegExpValidator(QRegExp("^\\([+-]?\\d+(\\.\\d*)?([eE]\\d*)?,[+-]?\\d+(\\.\\d*)?([eE]\\d*)?\\)$"),
                        _createSoftBodyItemsUi->lineEditPosition));
    _createSoftBodyItemsUi->lineEditSize->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createSoftBodyItemsUi->lineEditSize));
    _createSoftBodyItemsUi->lineEditSplit->setValidator(
                new QIntValidator(0, INT_MAX, _createSoftBodyItemsUi->lineEditSplit));
    _createSoftBodyItemsUi->lineEditBodyMass->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createSoftBodyItemsUi->lineEditBodyMass));
    _createSoftBodyItemsUi->lineEditBodyDamping->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createSoftBodyItemsUi->lineEditBodyDamping));
    _createSoftBodyItemsUi->lineEditYoungModulus->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createSoftBodyItemsUi->lineEditYoungModulus));

    connect(_createSoftBodyItemsDialog, SIGNAL(okClicked()), this, SLOT(createSoftBodyItemsApply()));

    _createSoftBodyItemsDialog->exec();

    delete _createSoftBodyItemsDialog; _createSoftBodyItemsDialog = 0;
    delete _createSoftBodyItemsUi; _createSoftBodyItemsUi = 0;
}

void SoftBodyMenuHandler::createSoftBodyItemsApply()
{
    Q_ASSERT(_createSoftBodyItemsUi && _createSoftBodyItemsDialog);

    bool ok;
    StepCore::Vector2d position = StepCore::stringToType<StepCore::Vector2d>(
                    _createSoftBodyItemsUi->lineEditPosition->text(), &ok);
    double size = _createSoftBodyItemsUi->lineEditSize->text().toDouble();
    int split = _createSoftBodyItemsUi->lineEditSplit->text().toInt();
    double bodyMass = _createSoftBodyItemsUi->lineEditBodyMass->text().toDouble();
    double youngModulus = _createSoftBodyItemsUi->lineEditYoungModulus->text().toDouble();
    double bodyDamping = _createSoftBodyItemsUi->lineEditBodyDamping->text().toDouble();


    _worldModel->beginMacro(i18n("Edit %1", softBody()->name()));
    _worldModel->beginUpdate();

    StepCore::ItemList items =
            softBody()->createSoftBodyItems(position, size, split, bodyMass, youngModulus, bodyDamping);

    const StepCore::ItemList::const_iterator end = items.end();
    for(StepCore::ItemList::const_iterator it = items.begin(); it != end; ++it) {
        _worldModel->addItem(*it, softBody());
    }

    _worldModel->endUpdate();
    _worldModel->endMacro();
}

//////////////////////////////////////////////////

SoftBodyGraphicsItem::SoftBodyGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::SoftBody*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);
    setZValue(BODY_ZVALUE-1);
    _velocityHandler = new ArrowHandlerGraphicsItem(item, worldModel, this,
                   _item->metaObject()->property("velocity"),
                   _item->metaObject()->property("position"));
    _velocityHandler->setVisible(false);
}

inline StepCore::SoftBody* SoftBodyGraphicsItem::softBody() const
{
    return static_cast<StepCore::SoftBody*>(_item);
}

QPainterPath SoftBodyGraphicsItem::shape() const
{
    return _painterPath;
}

void SoftBodyGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    int renderHints = painter->renderHints();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(Qt::darkGray));
    painter->drawPath(_painterPath);
    painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);

    if(_isSelected) {
        double s = currentViewScale();
        QRectF rect = _painterPath.boundingRect();
        rect.adjust(-SELECTION_MARGIN/s, -SELECTION_MARGIN/s, SELECTION_MARGIN/s, SELECTION_MARGIN/s);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(QBrush());
        painter->drawRect(rect);
        painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);
    }

    if(_isSelected || _isMouseOverItem) {
        painter->setPen(QPen(Qt::blue, 0));
        drawArrow(painter, softBody()->position(), softBody()->velocity());
        painter->setPen(QPen(Qt::red, 0));
        drawArrow(painter, softBody()->position(), softBody()->acceleration());
    }
}

void SoftBodyGraphicsItem::viewScaleChanged()
{
    prepareGeometryChange();

    _painterPath = QPainterPath();
    _painterPath.setFillRule(Qt::WindingFill);

    if(softBody()->borderParticles().size() > 0) {
        _painterPath.moveTo(vectorToPoint(
                dynamic_cast<StepCore::Particle*>(softBody()->borderParticles()[0])->position() ));
        for(unsigned int i=1; i<softBody()->borderParticles().size(); ++i) {
            _painterPath.lineTo(vectorToPoint(
                dynamic_cast<StepCore::Particle*>(softBody()->borderParticles()[i])->position() ));
        }
        _painterPath.closeSubpath();
    }

    double s = currentViewScale();

    _boundingRect = _painterPath.boundingRect();

    if(_isSelected || _isMouseOverItem) {
        const StepCore::Vector2d r = softBody()->position();
        const StepCore::Vector2d v = softBody()->velocity();
        const StepCore::Vector2d a = softBody()->acceleration();
        _boundingRect |= QRectF(r[0], r[1], v[0], v[1]).normalized()
                         | QRectF(r[0], r[1], a[0], a[1]).normalized();
    }

    double adjust = (ARROW_STROKE+SELECTION_MARGIN)/s;
    _boundingRect.adjust(-adjust,-adjust, adjust, adjust);
}

void SoftBodyGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    Q_UNUSED(dynamicOnly);
    //setPos(vectorToPoint(softBody()->position()));
    viewScaleChanged();
    update();
}

void SoftBodyGraphicsItem::stateChanged()
{
    if(_isSelected) _velocityHandler->setVisible(true);
    else _velocityHandler->setVisible(false);
    viewScaleChanged();
    update();
}
