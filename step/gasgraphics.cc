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

#include "gasgraphics.h"
#include "gasgraphics.moc"

#include "ui_create_gas_particles.h"

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

double GasCreator::random11()
{
    return double(qrand()) / (RAND_MAX/2) - 1;
}

bool GasCreator::sceneEvent(QEvent* event)
{
    if(event->type() == QEvent::GraphicsSceneMousePress) {
        _worldModel->simulationPause();

        _worldModel->beginMacro(i18n("Create %1", _className));
        _item = _worldModel->newItem(_className); Q_ASSERT(_item != NULL);
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);

        StepCore::Gas* gas = static_cast<StepCore::Gas*>(_item);
        _worldModel->newItem("GasLJForce", gas);
        StepCore::Object* ljforce = gas->items()[0];
        _worldModel->setProperty(ljforce, ljforce->metaObject()->property("depth"), 0.1);
        _worldModel->setProperty(ljforce, ljforce->metaObject()->property("rmin"), 0.1);

        _worldModel->endMacro();
        event->accept();
        return true;
    }
    return false;

}

class GasArrowHandlerGraphicsItem: public ArrowHandlerGraphicsItem
{
public:
    GasArrowHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                        QGraphicsItem* parent, const StepCore::MetaProperty* property)
        : ArrowHandlerGraphicsItem(item, worldModel, parent, property) {}

protected:
    StepCore::Vector2d value() {
        return static_cast<StepCore::Gas*>(_item)->measureRectCenter() +
                static_cast<StepCore::Gas*>(_item)->measureRectSize()/2.0;
    }
    void setValue(const StepCore::Vector2d& value) {
        _worldModel->simulationPause();
        StepCore::Vector2d v = (value - static_cast<StepCore::Gas*>(_item)->measureRectCenter())*2.0;
        _worldModel->setProperty(_item, _property, QVariant::fromValue(v));
    }
};

GasGraphicsItem::GasGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Gas*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    //setFlag(QGraphicsItem::ItemIsMovable);
    //setAcceptsHoverEvents(true);
    _centerHandler = new ArrowHandlerGraphicsItem(item, worldModel, this,
                            _item->metaObject()->property("measureRectCenter"));
    _sizeHandler = new GasArrowHandlerGraphicsItem(item, worldModel, this,
                            _item->metaObject()->property("measureRectSize"));
    _centerHandler->setVisible(false);
    _sizeHandler->setVisible(false);
    setZValue(HANDLER_ZVALUE);
}

inline StepCore::Gas* GasGraphicsItem::gas() const
{
    return static_cast<StepCore::Gas*>(_item);
}

QPainterPath GasGraphicsItem::shape() const
{
    QPainterPath path;
    //path.addRect(QRectF(-radius,-radius,radius*2,radius*2));
    return path;
}

void GasGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    if(_isSelected) {
        painter->setPen(QPen(Qt::red, 0));
        painter->drawRect(_boundingRect);
    }
}

void GasGraphicsItem::viewScaleChanged()
{
}

void GasGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) stateChanged();
}

void GasGraphicsItem::stateChanged()
{
    prepareGeometryChange();
    if(_isSelected) {
        const StepCore::Vector2d& size = gas()->measureRectSize();
        StepCore::Vector2d r0 = gas()->measureRectCenter() - size/2.0;
        _boundingRect = QRectF(r0[0], r0[1], size[0], size[1]);
        _centerHandler->setVisible(true);
        _sizeHandler->setVisible(true);
    }
    else {
        _boundingRect = QRectF();
        _centerHandler->setVisible(false);
        _sizeHandler->setVisible(false);
    }
    update();
}

void GasMenuHandler::populateMenu(QMenu* menu)
{
    _createGasParticlesUi = 0;
    _createGasParticlesDialog = 0;
    //_confChanged = false;

    menu->addAction(KIcon("configure"), i18n("Create particles..."), this, SLOT(createGasParticles()));
    //menu->addAction(KIcon("edit-clear"), i18n("Clear gas"), this, SLOT(clearGas()));
    menu->addSeparator();
    ItemMenuHandler::populateMenu(menu);
}

inline StepCore::Gas* GasMenuHandler::gas() const
{
    return static_cast<StepCore::Gas*>(_object);
}

void GasMenuHandler::clearGas()
{
//    _worldModel->simulationPause();

}

void GasMenuHandler::createGasParticles()
{
    if(_worldModel->isSimulationActive())
        _worldModel->simulationStop();

    _createGasParticlesDialog = new KDialog(); // XXX: parent?
    
    _createGasParticlesDialog->setCaption(i18n("Create gas particles"));
    _createGasParticlesDialog->setButtons(KDialog::Ok | KDialog::Cancel);

    _createGasParticlesUi = new Ui::WidgetCreateGasParticles;
    _createGasParticlesUi->setupUi(_createGasParticlesDialog->mainWidget());

    _createGasParticlesUi->lineEditMass->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createGasParticlesUi->lineEditMass));
    _createGasParticlesUi->lineEditCount->setValidator(
                new QIntValidator(0, 1000, _createGasParticlesUi->lineEditCount));
    _createGasParticlesUi->lineEditTemperature->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createGasParticlesUi->lineEditTemperature));

    connect(_createGasParticlesDialog, SIGNAL(okClicked()), this, SLOT(createGasParticlesApply()));

    _createGasParticlesDialog->exec();

    delete _createGasParticlesDialog; _createGasParticlesDialog = 0;
    delete _createGasParticlesUi; _createGasParticlesUi = 0;
}

void GasMenuHandler::createGasParticlesApply()
{
    Q_ASSERT(_createGasParticlesUi && _createGasParticlesDialog);

    _worldModel->beginMacro(i18n("Edit %1", gas()->name()));
    _worldModel->beginUpdate();

    int count = _createGasParticlesUi->lineEditCount->text().toInt();
    double mass = _createGasParticlesUi->lineEditMass->text().toDouble();
    double temperature = _createGasParticlesUi->lineEditTemperature->text().toDouble();
    std::vector<StepCore::GasParticle*> particles =
            gas()->rectCreateParticles(count, mass, temperature);


    const StepCore::GasParticleList::const_iterator end = particles.end();
    for(StepCore::GasParticleList::const_iterator it = particles.begin(); it != end; ++it) {
        _worldModel->addItem(*it, gas());
    }

    _worldModel->endUpdate();
    _worldModel->endMacro();
}

/*
void GasMenuHandler::clearGas()
{
    _worldModel->simulationPause();
    //_lastPointTime = -HUGE_VAL; // XXX
    _worldModel->setProperty(gas(), gas()->metaObject()->property("points"),
                               QVariant::fromValue(std::vector<StepCore::Vector2d>()) );
}
*/


