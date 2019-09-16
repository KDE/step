/* This file is part of Step.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
   Copyright (C) 2014 Inge Wallin        <inge@lysator.liu.se>

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

#include <stepcore/softbody.h>
#include <stepcore/types.h>

#include "ui_create_softbody_items.h"

#include <float.h>

#include "worldmodel.h"
#include "worldfactory.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QEvent>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QItemSelectionModel>
#include <QPainter>
#include <QPushButton>
#include <QRegularExpressionValidator>
#include <QVBoxLayout>

#include <KLocalizedString>


void SoftBodyCreator::start()
{
    showMessage(MessageFrame::Information,
            i18n("Click on the scene to create a %1", classNameTr()));
}

bool SoftBodyCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    
    if (event->type() == QEvent::GraphicsSceneMousePress &&
        mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        StepCore::Vector2d position = StepGraphicsItem::pointToVector(pos);
        
        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1",
                                     _worldModel->newItemName(_className)));

        showMessage(MessageFrame::Information,
            i18n("Please fill in the parameters for %1", classNameTr()));

        _item = _worldModel->createItem(_className); Q_ASSERT(_item != 0);
        
        SoftBodyMenuHandler* menuHandler =
            new SoftBodyMenuHandler(_item, _worldModel, 0);
        menuHandler->createSoftBodyItems(position);
        menuHandler->deleteLater();
        
        _worldModel->endMacro();

        if (menuHandler->applied()) {
            showMessage(MessageFrame::Information,
                i18n("%1 named '%2' created", classNameTr(), _item->name()),
                MessageFrame::CloseButton | MessageFrame::CloseTimer);
        }
        else {
            delete _item;
            _item = 0;
        }

        setFinished();
        
        return true;
    }
    
    return false;
}

void SoftBodyMenuHandler::populateMenu(QMenu* menu, KActionCollection* actions)
{
    _createSoftBodyItemsUi = 0;
    _createSoftBodyItemsDialog = 0;
    //_confChanged = false;

    // XXX: better icon
    // TODO: This will never work the way SoftBody is right now...
    //menu->addAction(QIcon::fromTheme("step_object_GasParticle"), i18n("Create items..."), this, SLOT(createSoftBodyItems()));
    //menu->addAction(QIcon::fromTheme("edit-clear"), i18n("Clear gas"), this, SLOT(clearGas()));
    menu->addSeparator();
    ItemMenuHandler::populateMenu(menu, actions);
}

inline StepCore::SoftBody* SoftBodyMenuHandler::softBody() const
{
    return static_cast<StepCore::SoftBody*>(_object);
}

void SoftBodyMenuHandler::clearSoftBody()
{
//    _worldModel->simulationPause();
}

void SoftBodyMenuHandler::createSoftBodyItems(const StepCore::Vector2d& pos)
{
    if(_worldModel->isSimulationActive())
        _worldModel->simulationStop();

    _createSoftBodyItemsDialog = new QDialog(); // XXX: parent?
    
    _createSoftBodyItemsDialog->setWindowTitle(i18n("Create soft body items"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout;
    _createSoftBodyItemsDialog->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    _createSoftBodyItemsDialog->connect(buttonBox, &QDialogButtonBox::accepted,
					_createSoftBodyItemsDialog, &QDialog::accept);
    _createSoftBodyItemsDialog->connect(buttonBox, &QDialogButtonBox::rejected,
					_createSoftBodyItemsDialog, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    _createSoftBodyItemsUi = new Ui::WidgetCreateSoftBodyItems;
    _createSoftBodyItemsUi->setupUi(mainWidget);

    _createSoftBodyItemsUi->lineEditPosition->setValidator(
                new QRegularExpressionValidator(QRegularExpression("^\\([+-]?\\d+(\\.\\d*)?([eE]\\d*)?,[+-]?\\d+(\\.\\d*)?([eE]\\d*)?\\)$"),
                        _createSoftBodyItemsUi->lineEditPosition));
    _createSoftBodyItemsUi->lineEditPosition->setText(StepCore::typeToString(pos));
    _createSoftBodyItemsUi->lineEditSize->setValidator(
                new QRegularExpressionValidator(QRegularExpression("^\\([+-]?\\d+(\\.\\d*)?([eE]\\d*)?,[+-]?\\d+(\\.\\d*)?([eE]\\d*)?\\)$"),
                        _createSoftBodyItemsUi->lineEditSize));
    _createSoftBodyItemsUi->lineEditSplit->setValidator(
                new QRegularExpressionValidator(QRegularExpression("^\\(\\d+,\\d+\\)$"),
                        _createSoftBodyItemsUi->lineEditSplit));
    _createSoftBodyItemsUi->lineEditBodyMass->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createSoftBodyItemsUi->lineEditBodyMass));
    _createSoftBodyItemsUi->lineEditBodyDamping->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createSoftBodyItemsUi->lineEditBodyDamping));
    _createSoftBodyItemsUi->lineEditYoungModulus->setValidator(
                new QDoubleValidator(0, HUGE_VAL, DBL_DIG, _createSoftBodyItemsUi->lineEditYoungModulus));

    int retval = _createSoftBodyItemsDialog->exec();
    if (retval == QDialog::Accepted) {
	createSoftBodyItemsApply();
    }

    delete _createSoftBodyItemsDialog; _createSoftBodyItemsDialog = 0;
    delete _createSoftBodyItemsUi; _createSoftBodyItemsUi = 0;
}

void SoftBodyMenuHandler::createSoftBodyItemsApply()
{
    Q_ASSERT(_createSoftBodyItemsUi && _createSoftBodyItemsDialog);

    bool ok;
    StepCore::Vector2d position = StepCore::stringToType<StepCore::Vector2d>(
                    _createSoftBodyItemsUi->lineEditPosition->text(), &ok);
    StepCore::Vector2d size = StepCore::stringToType<StepCore::Vector2d>(
                    _createSoftBodyItemsUi->lineEditSize->text(), &ok);
    StepCore::Vector2i split = StepCore::stringToType<StepCore::Vector2i>(
                    _createSoftBodyItemsUi->lineEditSplit->text(), &ok);

    double bodyMass = _createSoftBodyItemsUi->lineEditBodyMass->text().toDouble();
    double youngModulus = _createSoftBodyItemsUi->lineEditYoungModulus->text().toDouble();
    double bodyDamping = _createSoftBodyItemsUi->lineEditBodyDamping->text().toDouble();


    _worldModel->beginMacro(i18n("Create items for %1", softBody()->name()));

    StepCore::ItemList items =
            softBody()->createSoftBodyItems(position, size, split, bodyMass, youngModulus, bodyDamping);

    _worldModel->addItem(softBody());
    _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(softBody()),
                                                   QItemSelectionModel::ClearAndSelect);

    const StepCore::ItemList::const_iterator end = items.end();
    for(StepCore::ItemList::const_iterator it = items.begin(); it != end; ++it) {
        (*it)->setName(_worldModel->getUniqueName((*it)->metaObject()->className()));
        _worldModel->addItem(*it, softBody());
    }

    _worldModel->endMacro();

    _applied = true;
}

/////////////////////////////////////////////////

SoftBodyGraphicsItem::SoftBodyGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : StepGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::SoftBody*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true);
    setZValue(BODY_ZVALUE-1);
    _velocityHandler = new ArrowHandlerGraphicsItem(item, worldModel, this,
                   _item->metaObject()->property(QStringLiteral("velocity")),
                   _item->metaObject()->property(QStringLiteral("position")));
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
    painter->setBrush(QBrush(QColor::fromRgba(softBody()->color())));
    painter->drawPath(_painterPath);

    if(_isSelected) {
        double s = currentViewScale();
        QRectF rect = _painterPath.boundingRect();
        rect.adjust(-SELECTION_MARGIN/s, -SELECTION_MARGIN/s, SELECTION_MARGIN/s, SELECTION_MARGIN/s);
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(QBrush());
        painter->drawRect(rect);
    }

    painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);
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

void SoftBodyGraphicsItem::mouseSetPos(const QPointF& /*pos*/, const QPointF& diff, MovingState)
{
    _worldModel->simulationPause();
    _worldModel->setProperty(_item, QStringLiteral("position"),
                QVariant::fromValue((softBody()->position() + pointToVector(diff)).eval()));
}

void SoftBodyParticleGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) {
        StepCore::SoftBody* sb = dynamic_cast<StepCore::SoftBody*>(particle()->group());
        if(sb) {
            if(sb->showInternalItems() && !isVisible()) show();
            else if(!sb->showInternalItems() && isVisible()) hide();
        }
    }
    if(isVisible()) {
        ParticleGraphicsItem::worldDataChanged(dynamicOnly);
    }
}

void SoftBodySpringGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) {
        StepCore::SoftBody* sb = dynamic_cast<StepCore::SoftBody*>(spring()->group());
        if(sb) {
            if(sb->showInternalItems() && !isVisible()) show();
            else if(!sb->showInternalItems() && isVisible()) hide();
        }
    }
    if(isVisible()) {
        SpringGraphicsItem::worldDataChanged(dynamicOnly);
    }
}

