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

#include <stepcore/gas.h>

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
#include <KMessageBox>
#include <KLocale>

#include <float.h>

bool GasCreator::sceneEvent(QEvent* event)
{
    if(event->type() == QEvent::GraphicsSceneMousePress) {
        _worldModel->simulationPause();

        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
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
        painter->setPen(QPen(QColor::fromRgba(gas()->color()), 0));
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

class GasKDialog: public KDialog
{
public:
    GasKDialog(GasMenuHandler* handler, QWidget *parent=0, Qt::WFlags flags=0)
        : KDialog(parent, flags), _handler(handler) {}
protected slots:
    void slotButtonClicked(int button) {
        if(button == KDialog::Ok) {
            if(_handler->createGasParticlesApply()) accept();
        } else {
            KDialog::slotButtonClicked(button);
        }
    }
    GasMenuHandler* _handler;
};

void GasMenuHandler::populateMenu(QMenu* menu)
{
    _createGasParticlesUi = 0;
    _createGasParticlesDialog = 0;
    //_confChanged = false;

    menu->addAction(KIcon("step_object_GasParticle"), i18n("Create particles..."), this, SLOT(createGasParticles()));
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

    _createGasParticlesDialog = new GasKDialog(this); // XXX: parent?
    
    _createGasParticlesDialog->setCaption(i18n("Create gas particles"));
    _createGasParticlesDialog->setButtons(KDialog::Ok | KDialog::Cancel);

    _createGasParticlesUi = new Ui::WidgetCreateGasParticles;
    _createGasParticlesUi->setupUi(_createGasParticlesDialog->mainWidget());

    _createGasParticlesUi->lineEditMass->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createGasParticlesUi->lineEditMass));
    _createGasParticlesUi->lineEditCount->setValidator(
                new QIntValidator(0, INT_MAX, _createGasParticlesUi->lineEditCount));
    _createGasParticlesUi->lineEditConcentration->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createGasParticlesUi->lineEditConcentration));
    _createGasParticlesUi->lineEditTemperature->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createGasParticlesUi->lineEditTemperature));
    _createGasParticlesUi->lineEditMeanVelocity->setValidator(
                new QRegExpValidator(QRegExp("^\\([+-]?\\d+(\\.\\d*)?([eE]\\d*)?,[+-]?\\d+(\\.\\d*)?([eE]\\d*)?\\)$"),
                        _createGasParticlesUi->lineEditMeanVelocity));

    _createGasParticlesUi->lineEditVolume->setText(QString::number(gas()->rectVolume()));
    createGasParticlesCountChanged();

    _createGasParticlesUi->labelVolume->setText(gas()->metaObject()->property("rectVolume")->units());
    _createGasParticlesUi->labelCount->setText(gas()->metaObject()->property("rectParticleCount")->units());
    _createGasParticlesUi->labelConcentration->setText(gas()->metaObject()->property("rectConcentration")->units());
    _createGasParticlesUi->labelMass->setText(gas()->metaObject()->property("rectMeanParticleMass")->units());
    _createGasParticlesUi->labelTemperature->setText(gas()->metaObject()->property("rectTemperature")->units());
    _createGasParticlesUi->labelMeanVelocity->setText(gas()->metaObject()->property("rectMeanVelocity")->units());

    connect(_createGasParticlesUi->lineEditCount, SIGNAL(textEdited(const QString&)),
                this, SLOT(createGasParticlesCountChanged()));
    connect(_createGasParticlesUi->lineEditConcentration, SIGNAL(textEdited(const QString&)),
                this, SLOT(createGasParticlesConcentrationChanged()));

    connect(_createGasParticlesDialog, SIGNAL(okClicked()), this, SLOT(createGasParticlesApply()));

    _createGasParticlesDialog->exec();

    delete _createGasParticlesDialog; _createGasParticlesDialog = 0;
    delete _createGasParticlesUi; _createGasParticlesUi = 0;
}

void GasMenuHandler::createGasParticlesCountChanged()
{
    _createGasParticlesUi->lineEditConcentration->setText(QString::number(
                    _createGasParticlesUi->lineEditCount->text().toDouble() / gas()->rectVolume()
                ));
}

void GasMenuHandler::createGasParticlesConcentrationChanged()
{
    _createGasParticlesUi->lineEditCount->setText(QString::number(
                    round(_createGasParticlesUi->lineEditConcentration->text().toDouble() * gas()->rectVolume())
                ));
}

bool GasMenuHandler::createGasParticlesApply()
{
    Q_ASSERT(_createGasParticlesUi && _createGasParticlesDialog);

    int count = _createGasParticlesUi->lineEditCount->text().toInt();

    if(count > MAX_PARTICLES) {
        int ret = KMessageBox::warningContinueCancel(NULL, 
              i18n("You are trying to create a very large number of particles. "
                   "It will make simulation very slow. Do you want to continue ?"),
              i18n("Warning - Step"));
        if(ret != KMessageBox::Continue) return false;
    }

    double mass = _createGasParticlesUi->lineEditMass->text().toDouble();
    double temperature = _createGasParticlesUi->lineEditTemperature->text().toDouble();

    bool ok;
    StepCore::Vector2d meanVelocity = StepCore::stringToType<StepCore::Vector2d>(
                    _createGasParticlesUi->lineEditMeanVelocity->text(), &ok);

    _worldModel->beginMacro(i18n("Create particles for %1", gas()->name()));

    std::vector<StepCore::GasParticle*> particles =
            gas()->rectCreateParticles(count, mass, temperature, meanVelocity);


    const StepCore::GasParticleList::const_iterator end = particles.end();
    for(StepCore::GasParticleList::const_iterator it = particles.begin(); it != end; ++it) {
        _worldModel->addItem(*it, gas());
    }

    _worldModel->endMacro();

    return true;
}

/*
void GasMenuHandler::clearGas()
{
    _worldModel->simulationPause();
    //_lastPointTime = -HUGE_VAL; // XXX
    _worldModel->setProperty(gas(), gas()->metaObject()->property("points"),
                               QVariant::fromValue(StepCore::Vector2dList()) );
}
*/


