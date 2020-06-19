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

#include "worldmodel.h"

#include "clipboard.h"
#include "simulationthread.h"
#include "worldgraphics.h"

#include "settings.h"

#include "worldfactory.h"
#include <stepcore/world.h>
#include <stepcore/xmlfile.h>
#include <stepcore/eulersolver.h>
#include <stepcore/collisionsolver.h>
#include <stepcore/constraintsolver.h>
#include <stepcore/types.h>

#include <QApplication>
#include <QItemSelectionModel>
#include <QMenu>
#include <QTimer>
#include <QUndoStack>

#include <KLocalizedString>

class CommandEditProperty: public QUndoCommand
{
public:
    CommandEditProperty(WorldModel* worldModel, StepCore::Object* object,
                const StepCore::MetaProperty* property, const QVariant& newValue, bool merge);

    int id() const Q_DECL_OVERRIDE { return _merge ? 1 : -1; }
    bool mergeWith(const QUndoCommand* command) Q_DECL_OVERRIDE;

    void redo() Q_DECL_OVERRIDE;
    void undo() Q_DECL_OVERRIDE;

protected:
    /* It's important to properly compress commands
     *  or the stack becomes too big and slow */
    struct EditProperty {
        StepCore::Object* object;
        const StepCore::MetaProperty* property;
        QVariant oldValue;
        QVariant newValue;
    };

    bool _merge;
    WorldModel* _worldModel;
    QList<EditProperty> _commands;
    //QList<StepCore::Object*> _objects;
};

CommandEditProperty::CommandEditProperty(WorldModel* worldModel, StepCore::Object* object,
            const StepCore::MetaProperty* property, const QVariant& newValue, bool merge)
        : _merge(merge), _worldModel(worldModel)
{
    //qDebug() << "CommandEditProperty: " << object->name() << " " << property->name() << " " << newValue.toString() << endl;
    EditProperty p = { object, property, property->readVariant(object), newValue };
    _commands << p;// _objects << object;
}

void CommandEditProperty::redo()
{
    bool fullUpdate = false;
    foreach(const EditProperty& p, _commands) {
        if(p.newValue.type() != QVariant::String) p.property->writeVariant(p.object, p.newValue);
        else p.property->writeString(p.object, p.newValue.value<QString>());
        if(!p.property->isDynamic() || p.property->hasSideEffects()) fullUpdate = true;
    }
    //_worldModel->objectChanged(NULL);
    _worldModel->emitChanged(fullUpdate, true);
    //foreach(StepCore::Object* object, _objects) _worldModel->objectChanged(object);
}

void CommandEditProperty::undo()
{
    bool fullUpdate = false;
    foreach(const EditProperty& p, _commands) {
        p.property->writeVariant(p.object, p.oldValue);
        if(!p.property->isDynamic() || p.property->hasSideEffects()) fullUpdate = true;
    }
    //_worldModel->objectChanged(NULL);
    _worldModel->emitChanged(fullUpdate, true);
    //foreach(StepCore::Object* object, _objects) _worldModel->objectChanged(object);
}

bool CommandEditProperty::mergeWith(const QUndoCommand* command)
{
    const CommandEditProperty* cmd = dynamic_cast<const CommandEditProperty*>(command);
    Q_ASSERT(cmd != NULL);
    if(cmd->_commands.count() != 1) return false;

    const EditProperty& p1 = cmd->_commands[0];
    for(int i=0; i < _commands.count(); ++i) {
        if(_commands[i].object == p1.object && _commands[i].property == p1.property) {
            _commands[i].newValue = p1.newValue;
            //if(!_objects.contains(p1.object)) _objects << p1.object;
            return true;
        }
    }
    _commands << p1;
    //if(!_objects.contains(p1.object)) _objects << p1.object;
    return true;
}

class CommandNewItem: public QUndoCommand
{
public:
    CommandNewItem(WorldModel* worldModel, StepCore::Item* item, StepCore::ItemGroup* parent, bool create)
            : _worldModel(worldModel), _item(item), _parent(parent), _create(create), _shouldDelete(create) {
        if(!create) findLinks(item, static_cast<StepCore::ItemGroup*>(_worldModel->world()));
    }
    ~CommandNewItem() { if(_shouldDelete) delete _item; }

    void redo() Q_DECL_OVERRIDE;
    void undo() Q_DECL_OVERRIDE;

protected:
    void findLinks(StepCore::Item* itemToMatch, StepCore::ItemGroup* groupToSearch);
    void removeItem();
    void readdItem();

    WorldModel* _worldModel;
    StepCore::Item* _item;
    StepCore::ItemGroup* _parent;
    bool _create;
    bool _shouldDelete;

    typedef QPair<StepCore::Object*, const StepCore::MetaProperty*> Link;
    QList<Link> _links;
};

void CommandNewItem::findLinks(StepCore::Item* itemToMatch, StepCore::ItemGroup* groupToSearch)
{
    StepCore::ItemGroup* itemToDeleteGroup = dynamic_cast<StepCore::ItemGroup*>(_item);

    for (StepCore::Item *item : groupToSearch->items()) {
        if (item == itemToMatch) // no need to find links in itself
            continue;
        if (itemToDeleteGroup && itemToDeleteGroup->contains(item)) // no need to find links if the item is a child of the item that is being deleted
            continue;
        const StepCore::MetaObject* mobj = item->metaObject();
        for(int i=0; i<mobj->propertyCount(); ++i) {
            if(mobj->property(i)->userTypeId() != qMetaTypeId<StepCore::Object*>()) continue;
            if(itemToMatch == mobj->property(i)->readVariant(item).value<StepCore::Object*>())
                _links << qMakePair(static_cast<StepCore::Object*>(item), mobj->property(i));
        }
        if(mobj->inherits<StepCore::ItemGroup>())
            findLinks(itemToMatch, static_cast<StepCore::ItemGroup*>(item));
    }

    // If the item being deleted is a group, be sure we also find links for all its children
    StepCore::ItemGroup* itemToMatchGroup = dynamic_cast<StepCore::ItemGroup*>(itemToMatch);
    if (itemToMatchGroup) {
        for (StepCore::Item *child : itemToMatchGroup->items()) {
            findLinks(child, groupToSearch);
        }
    }
}

void CommandNewItem::removeItem()
{
    foreach(const Link& link, _links)
        link.second->writeVariant(link.first, QVariant::fromValue<StepCore::Object*>(NULL));
    //qDebug("%d links removed", _links.count());
    _worldModel->removeCreatedItem(_item);
}

void CommandNewItem::readdItem()
{
    _worldModel->addCreatedItem(_item, _parent);
    foreach(const Link& link, _links)
        link.second->writeVariant(link.first, QVariant::fromValue<StepCore::Object*>(_item));
    //qDebug("%d links restored", _links.count());
}

void CommandNewItem::redo()
{
    if(_create) readdItem();
    else removeItem();
    _shouldDelete = !_create;
}

void CommandNewItem::undo()
{
    if(_create) removeItem();
    else readdItem();
    _shouldDelete = _create;
}

class CommandSetSolver: public QUndoCommand
{
public:
    CommandSetSolver(WorldModel* worldModel, StepCore::Solver* solver)
            : _worldModel(worldModel), _solver(solver) {}
    ~CommandSetSolver() { delete _solver; }
    void redo() Q_DECL_OVERRIDE { _solver = _worldModel->swapSolver(_solver); }
    void undo() Q_DECL_OVERRIDE { _solver = _worldModel->swapSolver(_solver); }

protected:
    WorldModel* _worldModel;
    StepCore::Solver* _solver;
};

class CommandSimulate: public QUndoCommand
{
public:
    CommandSimulate(WorldModel* worldModel);
    ~CommandSimulate() { delete _worldCopy; }

    void done();

    void redo() Q_DECL_OVERRIDE;
    void undo() Q_DECL_OVERRIDE;

    const QString& startTime() const { return _startTime; }
    const QString& endTime() const { return _endTime; }

protected:
    typedef QPair<int, int> PairInt;
    PairInt indexToPair(const QModelIndex &index);
    QModelIndex pairToIndex(PairInt pair);

    WorldModel* _worldModel;
    StepCore::World* _worldCopy;
    StepCore::World* _oldWorld;
    StepCore::World* _newWorld;

    QString _startTime;
    QString _endTime;
};

CommandSimulate::PairInt CommandSimulate::indexToPair(const QModelIndex &index)
{
    if(index.parent().isValid()) return PairInt(1, index.row());
    else return PairInt(0, index.row());
}

QModelIndex CommandSimulate::pairToIndex(CommandSimulate::PairInt pair)
{
    if(pair.first == 0) {
        if(pair.second == 0) return _worldModel->worldIndex();
        else if(pair.second == 1) return _worldModel->solverIndex();
        else if(pair.second == 2) return _worldModel->collisionSolverIndex();
        else if(pair.second == 3) return _worldModel->constraintSolverIndex();
        else return QModelIndex();
    } else return _worldModel->childItemIndex(pair.second);
}

CommandSimulate::CommandSimulate(WorldModel* worldModel)
{
    _worldModel = worldModel;
    _oldWorld = _worldCopy = _worldModel->_world;

    /*
    QList<PairInt> selection;
    foreach(QModelIndex index, _worldModel->_selectionModel->selection().indexes())
        selection << indexToPair(index);
    PairInt current = indexToPair(_worldModel->_selectionModel->currentIndex());
    */

    _worldModel->beginResetModel();
    _newWorld = _worldModel->_world = new StepCore::World(*_oldWorld);
    _worldModel->endResetModel();

    _startTime = _worldModel->formatProperty(_worldModel->_world, NULL,
                            _worldModel->_world->metaObject()->property(QStringLiteral("time")),
                            WorldModel::FormatEditable);

    /*
    foreach(PairInt pair, selection)
        _worldModel->_selectionModel->select(pairToIndex(pair), QItemSelectionModel::Select);
    _worldModel->_selectionModel->setCurrentIndex(pairToIndex(current), QItemSelectionModel::Current);
    */
}

void CommandSimulate::done()
{
    _endTime = _worldModel->formatProperty(_worldModel->_world, NULL,
                            _worldModel->_world->metaObject()->property(QStringLiteral("time")),
                            WorldModel::FormatEditable);
}

void CommandSimulate::redo()
{
    if(_newWorld != _worldModel->_world) {
        _worldModel->beginResetModel();
        _worldModel->_world = _newWorld;
        _worldCopy = _oldWorld;

        _worldModel->endResetModel();
        _worldModel->_selectionModel->setCurrentIndex(
                _worldModel->worldIndex(), QItemSelectionModel::SelectCurrent);
    }
}

void CommandSimulate::undo()
{
    _worldModel->beginResetModel();
    _worldModel->_world = _oldWorld;
    _worldCopy = _newWorld;

    _worldModel->endResetModel();
    _worldModel->_selectionModel->setCurrentIndex(
            _worldModel->worldIndex(), QItemSelectionModel::SelectCurrent);
}

WorldModel::WorldModel(QObject* parent)
    : QAbstractItemModel(parent), _actions(0)
{
    _selectionModel = new QItemSelectionModel(this, this);
    _undoStack = new QUndoStack(this);
    _clipboard = new Clipboard(this);
    _worldFactory = new WorldFactory();
    _world = new StepCore::World();

    _simulationCommand = NULL;
    _simulationTimer = new QTimer(this);
    _simulationTimer0 = new QTimer(this);
    _simulationTimer0->setSingleShot(true);
    _simulationTimer0->setInterval(0);
    setSimulationFps(25); // XXX KConfig ?

    _simulationThread = new SimulationThread(&_world);
    _simulationThread->start();

    connect(_simulationTimer, &QTimer::timeout,
                this, &WorldModel::simulationFrameBegin);
    connect(_simulationTimer0, &QTimer::timeout,
                this, &WorldModel::simulationFrameBegin);
    connect(_simulationThread, &SimulationThread::worldEvolveDone,
                this, &WorldModel::simulationFrameEnd, Qt::QueuedConnection);

    _updatingTimer = new QTimer(this);
    _updatingTimer->setSingleShot(true);
    _updatingTimer->setInterval(0);
    _updatingFullUpdate = false;
    _updatingRecalcFn = false;

    connect(_updatingTimer, &QTimer::timeout,
                this, &WorldModel::doEmitChanged);

    resetWorld();

    _simulationFrameWaiting = false;
    _simulationFrameSkipped = false;
    _simulationStopping = false;
    _simulationPaused = false;

}

WorldModel::~WorldModel()
{
    delete _simulationThread;
    delete _worldFactory;
    delete _world;
}

void WorldModel::resetWorld()
{
    Q_ASSERT(!isSimulationActive());
    beginResetModel();
    if(_world->name().isEmpty()) {
        // XXX: check that loaded items has unique names !
        _world->setName(getUniqueName(QStringLiteral("world")));
    }
    if(NULL == _world->solver()) {
        _world->setSolver(new StepCore::AdaptiveEulerSolver());
        _world->solver()->setName(getUniqueName(QStringLiteral("solver")));
    }
    if(NULL == _world->collisionSolver()) {
        _world->setCollisionSolver(new StepCore::GJKCollisionSolver());
        _world->collisionSolver()->setName(getUniqueName(QStringLiteral("collisionSolver")));
    }
    if(NULL == _world->constraintSolver()) {
        _world->setConstraintSolver(new StepCore::CGConstraintSolver());
        _world->constraintSolver()->setName(getUniqueName(QStringLiteral("constraintSolver")));
    }
    _undoStack->clear();

    _world->doCalcFn();
    endResetModel();

    _selectionModel->setCurrentIndex(worldIndex(), QItemSelectionModel::SelectCurrent);

    emitChanged(true, false);
}

void WorldModel::emitChanged(bool fullUpdate, bool recalcFn)
{
    //qDebug() << "emitChanged(): " << world()->time() << endl;
    if(fullUpdate) _updatingFullUpdate = true;
    if(recalcFn) _updatingRecalcFn = true;
    if(!_updatingTimer->isActive()) _updatingTimer->start(0);
}

void WorldModel::doEmitChanged()
{
    if(_updatingRecalcFn) _world->doCalcFn();
    emit worldDataChanged(!_updatingFullUpdate);
    if(_updatingFullUpdate) {
        emit dataChanged(worldIndex(), constraintSolverIndex());
    }
}

QModelIndex WorldModel::worldIndex() const
{
    return createIndex(0, 0, _world);
}

QModelIndex WorldModel::solverIndex() const
{
    return createIndex(1, 0, _world->solver());
}

QModelIndex WorldModel::collisionSolverIndex() const
{
    return createIndex(2, 0, _world->collisionSolver());
}

QModelIndex WorldModel::constraintSolverIndex() const
{
    return createIndex(3, 0, _world->constraintSolver());
}

QModelIndex WorldModel::childItemIndex(int n, StepCore::ItemGroup* group) const
{
    if(!group) group = _world;
    return createIndex(n, 0, group->childItem(n));
}

QModelIndex WorldModel::objectIndex(StepCore::Object* obj) const
{
    if(obj == _world) return worldIndex();
    else if(obj == _world->solver()) return solverIndex();
    else if(obj == _world->collisionSolver()) return collisionSolverIndex();
    else if(obj == _world->constraintSolver()) return constraintSolverIndex();
    else {
        StepCore::Item* item = dynamic_cast<StepCore::Item*>(obj);
        for(StepCore::Item* it = item; it != _world; it = it->group()) {
            if(it == NULL) return QModelIndex();
        }
        STEPCORE_ASSERT_NOABORT(item && item->group());
        return createIndex(item->group()->childItemIndex(item), 0, item);
        //return itemIndex(_world->childItemIndex(dynamic_cast<const StepCore::Item*>(obj)));
    }
}

StepCore::Object* WorldModel::object(const QModelIndex& index) const
{
    if(index.isValid()) return static_cast<StepCore::Object*>(index.internalPointer());
    else return NULL;
}

#if 0
void WorldModel::objectChanged(const StepCore::Object* /*object*/)
{
    if(!_updating) {
        _world->doCalcFn();
        emitChanged(false);
    }
}
#endif

StepCore::Item* WorldModel::item(const QModelIndex& index) const
{
    return dynamic_cast<StepCore::Item*>(object(index));
}

QVariant WorldModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return QVariant();
    StepCore::Object* obj = static_cast<StepCore::Object*>(index.internalPointer());

    if(role == Qt::DisplayRole) {
        return formatNameFull(obj);
    } else if(role == Qt::ToolTipRole) {
        const_cast<WorldModel*>(this)->simulationPause();
        return createToolTip(index); // XXX
    } else if(role == Qt::DecorationRole) {
        if(_worldFactory->hasObjectIcon(obj->metaObject())) 
            return _worldFactory->objectIcon(obj->metaObject());
        return QVariant();
    } else if(role == FormattedNameRole) {
        return formatName(obj);
    } else if(role == ClassNameRole) {
        return obj->metaObject()->className();
    }

    return QVariant();
}

QModelIndex WorldModel::index(int row, int /*column*/, const QModelIndex &parent) const
{
    if(!parent.isValid()) {
        if(row == 0) return worldIndex();
        else if(row == 1) return solverIndex();
        else if(row == 2) return collisionSolverIndex();
        else if(row == 3) return constraintSolverIndex();
    } else {
        StepCore::ItemGroup* group = dynamic_cast<StepCore::ItemGroup*>(object(parent));
        if(group) return createIndex(row, 0, group->childItem(row));
        //if(parent.internalPointer() == _world) return itemIndex(row);
    }
    return QModelIndex();
}

QModelIndex WorldModel::parent(const QModelIndex &index) const
{
    if(!index.isValid()) return QModelIndex();
    else if(index.internalPointer() == _world) return QModelIndex();
    else if(index.internalPointer() == _world->solver()) return QModelIndex();
    else if(index.internalPointer() == _world->collisionSolver()) return QModelIndex();
    else if(index.internalPointer() == _world->constraintSolver()) return QModelIndex();
    else {
        StepCore::Item* item = dynamic_cast<StepCore::Item*>(object(index));
        if(item && item->group()) return objectIndex(item->group());
        else return QModelIndex();
    }
}

int WorldModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid()) {
        Q_ASSERT(_world->solver());
        Q_ASSERT(_world->collisionSolver());
        Q_ASSERT(_world->constraintSolver());
        return 4;
        /*
        int count = 1;
        if(_world->solver()) ++count;
        if(_world->collisionSolver()) ++count;
        return count;
        */
    }
    else if(dynamic_cast<StepCore::ItemGroup*>(object(parent)))
        return dynamic_cast<StepCore::ItemGroup*>(object(parent))->childItemCount();
    else return 0;
}

int WorldModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

QString WorldModel::newItemName(const QString& className)
{
    return getUniqueName(className);
}

StepCore::Item* WorldModel::createItem(const QString& className, StepCore::ItemGroup* parent)
{
    Q_UNUSED(parent)
    StepCore::Item* item = _worldFactory->newItem(className);
    if(item == NULL) return NULL;
    item->setName(getUniqueName(className));
    return item;
}

StepCore::Item* WorldModel::newItem(const QString& className, StepCore::ItemGroup* parent)
{
    StepCore::Item* item = _worldFactory->newItem(className);
    if(item == NULL) return NULL;
    item->setName(getUniqueName(className));
    pushCommand(new CommandNewItem(this, item, parent, true));
    return item;
}

void WorldModel::addItem(StepCore::Item* item, StepCore::ItemGroup* parent)
{
    //if(item->name().isEmpty()) item->setName(getUniqueName(item->metaObject()->className()));
    pushCommand(new CommandNewItem(this, item, parent, true));
}

StepCore::Solver* WorldModel::newSolver(const QString& className)
{
    StepCore::Solver* solver = _worldFactory->newSolver(className);
    if(solver == NULL) return NULL;

    // Copy similarly named properties
    // XXX: is it right ?
    StepCore::Solver* oldSolver = _world->solver();
    for(int i=0; i<oldSolver->metaObject()->propertyCount(); ++i) {
        const StepCore::MetaProperty* oldProperty = oldSolver->metaObject()->property(i);
        const StepCore::MetaProperty* newProperty = solver->metaObject()->property(oldProperty->name());
        if(newProperty && newProperty->isWritable()) {
            newProperty->writeVariant(solver, oldProperty->readVariant(oldSolver));
        }
    }

    //solver->setName(_world->solver()->name());
    //solver->setName(getUniqueName(name)); // XXX: is it better ?

    pushCommand(new CommandSetSolver(this, solver));
    return solver;
}

void WorldModel::deleteItem(StepCore::Item* item)
{
    pushCommand(new CommandNewItem(this, item, item->group(), false));
}

void WorldModel::deleteSelectedItems()
{
    simulationPause();

    QList<StepCore::Item*> items;
    foreach(QModelIndex index, selectionModel()->selectedIndexes()) {
        // Do not delete world item
        if (index == worldIndex()) continue;
        
        StepCore::Item* it = item(index);
        if (it) items << it;
    }

    foreach(StepCore::Item* it, items) {
        for(StepCore::Item* it1 = it->group(); it1 != 0 && it1 != _world; it1 = it1->group()) {
            if(items.contains(it1)) { items.removeOne(it); break; }
        }
    }

    if(!items.isEmpty()) {
        beginMacro(items.count()==1 ? i18n("Delete %1", items[0]->name())
                                    : i18n("Delete several items"));
        foreach(StepCore::Item* it, items) deleteItem(it);
        endMacro();
    }
}

void WorldModel::addCreatedItem(StepCore::Item* item, StepCore::ItemGroup* parent)
{
    if(!parent) parent = _world;
    beginInsertRows(objectIndex(parent), parent->childItemCount(), parent->childItemCount());
    parent->addItem(item);
    endInsertRows();
    emitChanged(true, true);
}

void WorldModel::removeCreatedItem(StepCore::Item* item)
{
    STEPCORE_ASSERT_NOABORT(item->group());
    QModelIndex index = objectIndex(item);
    beginRemoveRows(index.parent(), index.row(), index.row());
    item->group()->removeItem(item);
    endRemoveRows();
    emitChanged(true, true);
}

StepCore::Solver* WorldModel::swapSolver(StepCore::Solver* solver)
{
    bool selected = selectionModel()->isSelected(solverIndex());
    bool current = selectionModel()->currentIndex() == solverIndex();
    StepCore::Solver* oldSolver = _world->removeSolver();
    _world->setSolver(solver);
    if(selected) selectionModel()->select(solverIndex(), QItemSelectionModel::Select);
    if(current) selectionModel()->setCurrentIndex(solverIndex(), QItemSelectionModel::Current);
    emitChanged(true, true);
    return oldSolver;
}

void WorldModel::pushCommand(QUndoCommand* command)
{
    Q_ASSERT(!_simulationFrameWaiting || _simulationPaused);
    if(!isSimulationActive()) {
        _undoStack->push(command);
    } else {
        command->redo();
        delete command;
    }
}

void WorldModel::beginMacro(const QString& text)
{
    // XXX: simulation could be started (by hotkey) during macro creation
    if(!isSimulationActive())
        _undoStack->beginMacro(text);
}

void WorldModel::endMacro()
{
    // XXX: simulation could be stopped during macro creation
    if(!isSimulationActive())
        _undoStack->endMacro();
}

void WorldModel::setProperty(StepCore::Object* object,
            const StepCore::MetaProperty* property, const QVariant& value, UndoFlags flags)
{
    Q_ASSERT(object != NULL); Q_ASSERT(property != NULL);
    pushCommand(new CommandEditProperty(this, object, property, value, !flags.testFlag(UndoNoMerge)));
}

QString WorldModel::formatName(const StepCore::Object* object) const
{
    if(!object) return i18n("<no object>");
    else if(!object->name().isEmpty()) return object->name();
    return i18n("<unnamed>");
}

QString WorldModel::formatNameFull(const StepCore::Object* object) const
{
    if(!object) return i18n("<no object>");
    return i18n("%1: %2", formatName(object), object->metaObject()->classNameTr());
}

QString WorldModel::formatProperty(const StepCore::Object* object,
                                   const StepCore::Object* objectErrors,
                                   const StepCore::MetaProperty* property,
                                   FormatFlags flags) const
{
    int pr = Settings::floatDisplayPrecision();

    QString units;
    if(!flags.testFlag(FormatHideUnits)) {
        if(!flags.testFlag(FormatEditable) && !property->units().isEmpty()) 
            units.append(" [").append(property->units()).append("]");
#ifdef STEP_WITH_UNITSCALC
        else if(flags.testFlag(FormatEditable) && !property->units().isEmpty()
                    && property->userTypeId() == QMetaType::Double) 
            units.append(" ").append(property->units());
#endif
    }

    const StepCore::MetaProperty* pv = objectErrors ?
            objectErrors->metaObject()->property(property->name() + "Variance") : NULL;

    // Common property types
    if(property->userTypeId() == QMetaType::Double) {
        QString error;
        if(pv) error = QStringLiteral(" ± %1")
                        .arg(sqrt(pv->readVariant(objectErrors).toDouble()), 0, 'g', pr)
                        .append(units);
        return QString::number(property->readVariant(object).toDouble(), 'g', pr).append(units).append(error);
    } else if(property->userTypeId() == qMetaTypeId<StepCore::Vector2d>()) {
        QString error;
        if(pv) {
            StepCore::Vector2d vv = pv->readVariant(objectErrors).value<StepCore::Vector2d>();
            error = QStringLiteral(" ± (%1,%2)")
                        .arg(sqrt(vv[0]), 0, 'g', pr).arg(sqrt(vv[1]), 0, 'g', pr).append(units);
        }
        StepCore::Vector2d v = property->readVariant(object).value<StepCore::Vector2d>();
        return QStringLiteral("(%1,%2)").arg(v[0], 0, 'g', pr).arg(v[1], 0, 'g', pr).append(units).append(error);
    } else if(property->userTypeId() == qMetaTypeId<StepCore::Vector2dList >() ) {
        // XXX: add error information
//         if(pv) qDebug() << "Unhandled property variance type" << endl;
        StepCore::Vector2dList list =
                property->readVariant(object).value<StepCore::Vector2dList >();
        QString string;
        unsigned int end = flags.testFlag(FormatEditable) ? list.size() : qMin<unsigned int>(10, list.size());
        for(unsigned int i=0; i<end; ++i) {
            if(!string.isEmpty()) string += ',';
            string += QStringLiteral("(%1,%2)").arg(list[i][0], 0, 'g', pr)
                                            .arg(list[i][1], 0, 'g', pr);
        }
        if(!flags.testFlag(FormatEditable) && end != 0 && end < list.size()) string += QLatin1String(",...");
        string.append(units);
        return string;
    } else {
        // default type
        // XXX: add error information
        //if(pe) error = QString::fromUtf8(" ± ").append(pe->readString(_objectErrors)).append(units);
        //if(pv) qDebug() << "Unhandled property variance type" << endl;
        Q_ASSERT(!pv);
        QString str = property->readString(object);
        if(!flags.testFlag(FormatEditable) && str.length() > 50)
            str = str.left(50).append("...");
        return str.append(units);
    }
}

QString WorldModel::createToolTip(const QModelIndex& index) const
{
    //Q_ASSERT(object != NULL);
    Q_ASSERT(index.isValid());
    QString toolTip = i18n("<nobr><h4><u>%1</u></h4></nobr>", index.data(Qt::DisplayRole).toString());
    toolTip += QLatin1String("<table>");

    StepCore::Object* object = this->object(index);
    StepCore::Item* item = dynamic_cast<StepCore::Item*>(object);
    const StepCore::Object* objectErrors = NULL;
    if(item) {
        if(world()->errorsCalculation()) objectErrors = item->objectErrors();
        else objectErrors = item->tryGetObjectErrors();
    }

    for(int i=0; i<object->metaObject()->propertyCount(); ++i) {
        const StepCore::MetaProperty* p = object->metaObject()->property(i);
        /*QString units;
        if(!p->units().isEmpty())
            units.append(" [").append(p->units()).append("]");
        QString value = p->readString(object);
        if(p->userTypeId() == qMetaTypeId<StepCore::Vector2dList >()) {
            // XXX: don't use readString in this case !
            value.replace("),(", QString(")%1<br />(").arg(units));
            value.append(units);
            if(value.count("<br />") > 10) {
                value = value.section("<br />", 0, 9);
                value.append("<br />...");
            }
        } else {
            value.append(units);
        }*/
        toolTip += i18n("<tr><td>%1&nbsp;&nbsp;</td><td>%2</td></tr>", p->nameTr(),
                            formatProperty(object, objectErrors, p));
    }
    toolTip += QLatin1String("</table>");
    //qDebug("%s", toolTip.toAscii().constData());
    return toolTip;
}

QMenu* WorldModel::createContextMenu(const QModelIndex& index)
{
    //Q_ASSERT(object != NULL);
    Q_ASSERT(index.isValid());
    QMenu* menu = new QMenu();
    menu->addSection(index.data(Qt::DisplayRole).toString());
    ItemMenuHandler* handler = _worldFactory->newItemMenuHandler(object(index), this, menu);
    handler->populateMenu(menu, _actions);
    return menu;
}

void WorldModel::clearWorld()
{
    _world->clear();
    resetWorld();
}

bool WorldModel::saveXml(QIODevice* device)
{
    StepCore::XmlFile file(device);

    if(file.save(_world)) {
        return true;
    } else {
        _errorString = file.errorString();
        return false;
    }
}

bool WorldModel::loadXml(QIODevice* device)
{
    _world->clear();
    StepCore::XmlFile file(device);
    bool ret = file.load(_world, _worldFactory);
    if(!ret) { _world->clear(); _errorString = file.errorString(); }
    resetWorld();
    return ret;
}

bool WorldModel::checkUniqueName(const QString& name) const
{
    if(name.isEmpty()) return false;
    if(_world->object(name) != NULL) return false;
    return true;
}

QString WorldModel::getUniqueName(const QString& className) const
{
    QString classNameCopy = className;
    classNameCopy[0] = classNameCopy[0].toLower();
    for(int n=1; ; ++n) {
        QString name = classNameCopy + QString::number(n);
        if(checkUniqueName(name)) return name;
    }
    return QString();
}

void WorldModel::simulationPause()
{
    if(!_simulationFrameWaiting || _simulationPaused) return;

    // Try to lock but do not wait mode than one frame
    if(_simulationThread->mutex()->tryLock(1000/_simulationFps)) {
        // We still need to setAbortEvolve(true) since
        // _simulationThread->doWorldEvolve() could be called before
        _world->setEvolveAbort(true);
    } else {
        //qDebug() << "simulationPause: simulation aborted" << endl;
        Q_ASSERT(isSimulationActive());
        Q_ASSERT(_simulationCommand);
        Q_ASSERT(_simulationFrameWaiting);
        //Q_ASSERT(!_simulationStopping);

        _world->setEvolveAbort(true);

        // Mutex will be locked as soon as current frame is aborted
        _simulationThread->mutex()->lock();
    }
    // We can release mutex just now since new simulation frame can
    // only be started from this thread when control returns to event loop
    _simulationThread->mutex()->unlock();
    _simulationPaused = true;

    //qDebug() << "simulationPause!" << endl;
    // XXX: do we need to reset evolveAbort here (and add threadAbort var) ?
    // XXX: do we need to call emitChanged here ?
}

void WorldModel::setSimulationFps(int simulationFps)
{
    _simulationFps = simulationFps;
    _simulationTimer->setInterval(1000/simulationFps);
}

bool WorldModel::isSimulationActive()
{
    return _simulationTimer->isActive();
}

void WorldModel::simulationStart()
{
    Q_ASSERT(!isSimulationActive());
    Q_ASSERT(!_simulationCommand);

    //_undoStack->beginMacro(i18n("Simulate"));
    _simulationCommand = new CommandSimulate(this);
    _world->setEvolveAbort(false);
    _simulationFrameWaiting = false;
    _simulationFrameSkipped = false;
    _simulationStopping = false;
    _simulationPaused = false;

    _simulationFrames = 0;
    _simulationStartTime = QTime::currentTime();
    _simulationTimer->start();

    //emitChanged();
}

void WorldModel::simulationStop()
{
    Q_ASSERT(isSimulationActive());
    Q_ASSERT(_simulationCommand);
    _simulationStopping = true;
    if(_simulationFrameWaiting) {
        simulationPause();
    } else {
        simulationFrameEnd(0);
    }
}

void WorldModel::simulationFrameBegin()
{
    Q_ASSERT(isSimulationActive());
    Q_ASSERT(_simulationCommand);
    Q_ASSERT(!_simulationStopping);

    //qDebug() << "simulationFrameBegin(): " << world()->time() << endl;

    if(_simulationFrameWaiting) { // TODO: warn user
        _simulationFrameSkipped = true;
        return;
    }

    if(_updatingTimer->isActive()) {
        // Wait for updating
        _simulationTimer0->start();
        return;
    }

    //qDebug("emit simulationDoFrame() t=%#x", int(QThread::currentThread()));
    //qDebug() << "simulationFrameBegin" << endl;
    _simulationFrameWaiting = true;
    _simulationPaused = false;
    _simulationThread->doWorldEvolve(1.0/_simulationFps);
    //qDebug("emitted simulationDoFrame()");
}

void WorldModel::simulationFrameEnd(int result)
{
    Q_ASSERT(isSimulationActive());
    Q_ASSERT(_simulationCommand);
    Q_ASSERT(_simulationFrameWaiting || _simulationStopping);

    ++_simulationFrames;
    if(_simulationFrames == 50) {
        qDebug("FPS: %f", double(_simulationFrames) /
                    _simulationStartTime.msecsTo(QTime::currentTime()) * 1000);
        _simulationStartTime = QTime::currentTime();
        _simulationFrames = 0;
    }

    //qDebug() << "simulationFrameEnd" << endl;

    // It's OK to be aborted
    if(result == StepCore::Solver::Aborted) {
        //qDebug("simulation frame aborted!");
        result = StepCore::Solver::OK;
    }

    // If current frame was aborted we can resume it now
    _world->setEvolveAbort(false);

    // Update GUI
    _simulationFrameWaiting = false;
    emitChanged(false, false);

    // Stop if requested or simulation error occurred
    if(_simulationStopping || result != StepCore::Solver::OK) {
        _simulationCommand->done();
        _undoStack->beginMacro(i18n("Simulate %1 → %2",
                _simulationCommand->startTime(), _simulationCommand->endTime()));
        _undoStack->push(_simulationCommand);
        _undoStack->endMacro();
        _simulationCommand = NULL;

        _simulationTimer->stop();
        _simulationTimer0->stop();
        emit simulationStopped(result);
        return;
    }

    if(_simulationFrameSkipped) {
        _simulationFrameSkipped = false;
        _simulationTimer0->start();
        //QApplication::processEvents(); // XXX
        //if(isSimulationActive() && !_simulationFrameWaiting)
        //    simulationFrameBegin();
    }
}

QList<StepCore::Item*> WorldModel::selectedItems()
{
    QList<StepCore::Item*> items;
    foreach (QModelIndex index, selectionModel()->selectedIndexes()) {
        // Do not delete world item
        if (index == worldIndex()) continue;
        
        StepCore::Item* it = item(index);
        if (it) items << it;
    }
    
    foreach (StepCore::Item* it, items) {
        for (StepCore::Item* it1 = it->group(); it1 != 0 && it1 != _world; it1 = it1->group()) {
            if (items.contains(it1)) {
                items.removeOne(it);
                break;
            }
        }
    }
    
    return items;
}

void WorldModel::cutSelectedItems()
{
    simulationPause();
    
    QList<StepCore::Item*> items = selectedItems();
    
    _clipboard->copy(items);
    
    if (!items.isEmpty()) {
        beginMacro(items.count() == 1 ? i18n("Cut %1", items[0]->name()) :
                   i18n("Cut several items"));
        foreach (StepCore::Item* it, items) deleteItem(it);
        endMacro();
    }
}

void WorldModel::copySelectedItems()
{
    simulationPause();
    
    QList<StepCore::Item*> items = selectedItems();
    
    _clipboard->copy(items);
}

void WorldModel::pasteItems()
{
    simulationPause();
    
    QList<StepCore::Item*> items = _clipboard->paste(_worldFactory);
    if (items.isEmpty()) return;
    
    beginMacro(items.count() == 1 ? i18n("Pasted %1", items[0]->name()) :
               i18n("Pasted several items"));
    QItemSelection selection;
    foreach (StepCore::Item* item, items) {
        QString className = item->metaObject()->className();
        item->setName(getUniqueName(className));
        addItem(item, _world);
        QModelIndex index = objectIndex(item);
        selection.select(index, index);
    }
    if (!selection.isEmpty()) {
        selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
    }
    endMacro();
}
