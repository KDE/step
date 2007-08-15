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
#include "simulationthread.h"
#include "worldgraphics.h"
#include "worldmodel.moc"

#include "worldfactory.h"
#include <stepcore/world.h>
#include <stepcore/xmlfile.h>
#include <stepcore/eulersolver.h>
#include <stepcore/collisionsolver.h>
#include <stepcore/types.h>
#include <QApplication>
#include <QItemSelectionModel>
#include <QUndoStack>
#include <QTimer>
#include <KMenu>
#include <KLocale>

class CommandEditProperty: public QUndoCommand
{
public:
    CommandEditProperty(WorldModel* worldModel, StepCore::Object* object,
                const StepCore::MetaProperty* property, const QVariant& newValue, bool merge);

    int id() const { return _merge ? 1 : -1; }
    bool mergeWith(const QUndoCommand* command);

    void redo();
    void undo();

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
    //kDebug() << "CommandEditProperty: " << object->name() << " " << property->name() << " " << newValue.toString() << endl;
    EditProperty p = { object, property, property->readVariant(object), newValue };
    _commands << p;// _objects << object;
}

void CommandEditProperty::redo()
{
    bool dynamicOnly = true;
    foreach(const EditProperty& p, _commands) {
        if(p.newValue.type() != QVariant::String) p.property->writeVariant(p.object, p.newValue);
        else p.property->writeString(p.object, p.newValue.value<QString>());
        if(!p.property->isDynamic() || p.property->hasSideEffects()) dynamicOnly = false;
    }
    //_worldModel->objectChanged(NULL);
    _worldModel->emitChanged(dynamicOnly);
    //foreach(StepCore::Object* object, _objects) _worldModel->objectChanged(object);
}

void CommandEditProperty::undo()
{
    bool dynamicOnly = true;
    foreach(const EditProperty& p, _commands) {
        p.property->writeVariant(p.object, p.oldValue);
        if(!p.property->isDynamic()) dynamicOnly = false;
    }
    //_worldModel->objectChanged(NULL);
    _worldModel->emitChanged(dynamicOnly);
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
        : _worldModel(worldModel), _item(item), _parent(parent), _create(create), _shouldDelete(create) {}
    ~CommandNewItem() { if(_shouldDelete) delete _item; }
    void redo();
    void undo();
protected:
    WorldModel* _worldModel;
    StepCore::Item* _item;
    StepCore::ItemGroup* _parent;
    bool _create;
    bool _shouldDelete;
};

void CommandNewItem::redo()
{
    if(_create) _worldModel->addItem(_item, _parent);
    else _worldModel->removeItem(_item);
    _shouldDelete = !_create;
}

void CommandNewItem::undo()
{
    if(_create) _worldModel->removeItem(_item);
    else _worldModel->addItem(_item, _parent);
    _shouldDelete = _create;
}

class CommandSetSolver: public QUndoCommand
{
public:
    CommandSetSolver(WorldModel* worldModel, StepCore::Solver* solver)
            : _worldModel(worldModel), _solver(solver) {}
    ~CommandSetSolver() { delete _solver; }
    void redo() { _solver = _worldModel->swapSolver(_solver); }
    void undo() { _solver = _worldModel->swapSolver(_solver); }

protected:
    WorldModel* _worldModel;
    StepCore::Solver* _solver;
};

class CommandSimulate: public QUndoCommand
{
public:
    CommandSimulate(WorldModel* worldModel);
    ~CommandSimulate() { delete _worldCopy; }
    void redo();
    void undo();

protected:
    typedef QPair<int, int> PairInt;
    PairInt indexToPair(QModelIndex index);
    QModelIndex pairToIndex(PairInt pair);

    WorldModel* _worldModel;
    StepCore::World* _worldCopy;
    StepCore::World* _oldWorld;
    StepCore::World* _newWorld;
};

CommandSimulate::PairInt CommandSimulate::indexToPair(QModelIndex index)
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

    _newWorld = _worldModel->_world = new StepCore::World(*_oldWorld);
    _worldModel->reset();

    /*
    foreach(PairInt pair, selection)
        _worldModel->_selectionModel->select(pairToIndex(pair), QItemSelectionModel::Select);
    _worldModel->_selectionModel->setCurrentIndex(pairToIndex(current), QItemSelectionModel::Current);
    */
}

void CommandSimulate::redo()
{
    if(_newWorld != _worldModel->_world) {
        _worldModel->_world = _newWorld;
        _worldCopy = _oldWorld;

        _worldModel->reset();
        _worldModel->_selectionModel->setCurrentIndex(
                _worldModel->worldIndex(), QItemSelectionModel::SelectCurrent);
    }
}

void CommandSimulate::undo()
{
    _worldModel->_world = _oldWorld;
    _worldCopy = _newWorld;

    _worldModel->reset();
    _worldModel->_selectionModel->setCurrentIndex(
            _worldModel->worldIndex(), QItemSelectionModel::SelectCurrent);
}

WorldModel::WorldModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    _selectionModel = new QItemSelectionModel(this, this);
    _undoStack = new QUndoStack(this);
    _worldFactory = new WorldFactory();
    _world = new StepCore::World();
    _simulationCommand = NULL;
    _simulationTimer = new QTimer(this);
    setSimulationFps(25); // XXX KConfig ?

    _simulationThread = new SimulationThread(&_world);
    _simulationThread->start();

    _updating = 0;
    _updatingDynamicOnly = true;
    resetWorld();

    _simulationFrameWaiting = false;
    _simulationFrameSkipped = false;
    _simulationStopping = false;
    _simulationPaused = false;

    QObject::connect(_simulationTimer, SIGNAL(timeout()),
                        this, SLOT(simulationFrameBegin()));
    QObject::connect(_simulationThread, SIGNAL(worldEvolveDone(int)),
                        this, SLOT(simulationFrameEnd(int)), Qt::QueuedConnection);
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
    if(_world->name().isEmpty()) {
        // XXX: check that loaded items has unique names !
        _world->setName(getUniqueName("world"));
    }
    if(NULL == _world->solver()) {
        _world->setSolver(new StepCore::AdaptiveEulerSolver());
        _world->solver()->setName(getUniqueName("solver"));
    }
    if(NULL == _world->collisionSolver()) {
        _world->setCollisionSolver(new StepCore::GJKCollisionSolver());
        _world->collisionSolver()->setName(getUniqueName("collisionSolver"));
    }
    _undoStack->clear();

    _world->doCalcFn();
    reset();

    _selectionModel->setCurrentIndex(worldIndex(), QItemSelectionModel::SelectCurrent);

    emitChanged();
}

void WorldModel::emitChanged(bool dynamicOnly)
{
    if(!_updating) {
        _world->doCalcFn();
        emit worldDataChanged(dynamicOnly);
        if(!dynamicOnly) {
            emit dataChanged(worldIndex(), collisionSolverIndex());
        }
    } else if(!dynamicOnly) {
        _updatingDynamicOnly = false;
    }
}

void WorldModel::endUpdate()
{
    if(!--_updating) {
        emitChanged(_updatingDynamicOnly);
        _updatingDynamicOnly = true;
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
    else {
        StepCore::Item* item = dynamic_cast<StepCore::Item*>(obj);
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
        return QString("%1: %2").arg(obj->name().isEmpty() ? i18n("<unnamed>") : obj->name())
                                   .arg(obj->metaObject()->className());
    } else if(role == Qt::ToolTipRole) {
        const_cast<WorldModel*>(this)->simulationPause();
        return createToolTip(index); // XXX
    } else if(role == ObjectNameRole) {
        return obj->name();
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
        return 3;
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

StepCore::Item* WorldModel::newItem(const QString& name, StepCore::ItemGroup* parent)
{
    StepCore::Item* item = _worldFactory->newItem(name);
    if(item == NULL) return NULL;
    item->setName(getUniqueName(name));
    pushCommand(new CommandNewItem(this, item, parent, true));
    return item;
}
        
StepCore::Solver* WorldModel::newSolver(const QString& name)
{
    StepCore::Solver* solver = _worldFactory->newSolver(name);
    if(solver == NULL) return NULL;

    // Copy similary named properties
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
        StepCore::Item* it = item(index); if(it) items << it;
    }
    if(!items.isEmpty()) {
        beginMacro(items.count()==1 ? i18n("Delete %1", items[0]->metaObject()->className())
                                    : i18n("Delete items"));
        foreach(StepCore::Item* it, items) deleteItem(it);
        endMacro();
    }
}

void WorldModel::addItem(StepCore::Item* item, StepCore::ItemGroup* parent)
{
    if(!parent) parent = _world;
    beginInsertRows(objectIndex(parent), parent->childItemCount(), parent->childItemCount());
    parent->addItem(item);
    endInsertRows();
    emitChanged();
}

void WorldModel::removeItem(StepCore::Item* item)
{
    STEPCORE_ASSERT_NOABORT(item->group());
    QModelIndex index = objectIndex(item);
    beginRemoveRows(index.parent(), index.row(), index.row());
    item->group()->removeItem(item);
    endRemoveRows();
    emitChanged();
}

StepCore::Solver* WorldModel::swapSolver(StepCore::Solver* solver)
{
    bool selected = selectionModel()->isSelected(solverIndex());
    bool current = selectionModel()->currentIndex() == solverIndex();
    beginRemoveRows(QModelIndex(), 1, 1);
    StepCore::Solver* oldSolver = _world->removeSolver();
    endRemoveRows();
    beginInsertRows(QModelIndex(), 1, 1);
    _world->setSolver(solver);
    endInsertRows();
    if(selected) selectionModel()->select(solverIndex(), QItemSelectionModel::Select);
    if(current) selectionModel()->setCurrentIndex(solverIndex(), QItemSelectionModel::Current);
    emitChanged();
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
            const StepCore::MetaProperty* property, const QVariant& value, bool merge)
{
    Q_ASSERT(object != NULL); Q_ASSERT(property != NULL);
    pushCommand(new CommandEditProperty(this, object, property, value, merge));
}

QString WorldModel::createToolTip(const QModelIndex& index) const
{
    //Q_ASSERT(object != NULL);
    Q_ASSERT(index.isValid());
    QString toolTip = i18n("<nobr><h4><u>%1</u></h4></nobr>", index.data(Qt::DisplayRole).toString());
    toolTip += "<table>";
    StepCore::Object* object = this->object(index);
    for(int i=0; i<object->metaObject()->propertyCount(); ++i) {
        const StepCore::MetaProperty* p = object->metaObject()->property(i);
        QString value = p->readString(object);
        if(p->userTypeId() == qMetaTypeId<std::vector<StepCore::Vector2d> >()) {
            value.replace("),(", ")<br />(");
            if(value.count("<br />") > 10) {
                value = value.section("<br />", 0, 9);
                value.append("<br />...");
            }
        }
        toolTip += i18n("<tr><td>%1&nbsp;&nbsp;</td><td>%2</td></tr>", p->name(), value);
    }
    toolTip += "</table>";
    //qDebug("%s", toolTip.toAscii().constData());
    return toolTip;
}

QMenu* WorldModel::createContextMenu(const QModelIndex& index)
{
    //Q_ASSERT(object != NULL);
    Q_ASSERT(index.isValid());
    KMenu* menu = new KMenu();
    menu->addTitle(index.data(Qt::DisplayRole).toString());
    ItemMenuHandler* handler = _worldFactory->newItemMenuHandler(object(index), this, menu);
    handler->populateMenu(menu);
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
        _undoStack->setClean();
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

QString WorldModel::getUniqueName(QString className) const
{
    className[0] = className[0].toLower();
    for(int n=1; ; ++n) {
        QString name = className + QString::number(n);
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
        //kDebug() << "simulationPause: simulation aborted" << endl;
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

    _undoStack->beginMacro(i18n("Simulate"));
    _simulationCommand = new CommandSimulate(this);
    _world->setEvolveAbort(false);
    _simulationFrameWaiting = false;
    _simulationFrameSkipped = false;
    _simulationStopping = false;
    _simulationPaused = false;

    _simulationTimer->start();

    emitChanged();
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

    if(_simulationFrameWaiting) { // TODO: warn user
        //qDebug("frame skipped!");
        _simulationFrameSkipped = true;
        return;
    }

    //qDebug("emit simulationDoFrame() t=%#x", int(QThread::currentThread()));
    //kDebug() << "simulationFrameBegin" << endl;
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

    //kDebug() << "simulationFrameEnd" << endl;

    // It's OK to be aborted
    if(result == StepCore::Solver::Aborted) {
        //qDebug("simulation frame aborted!");
        result = StepCore::Solver::OK;
    }

    // If current frame was aborted we can resume it now
    _world->setEvolveAbort(false);

    // Update GUI
    _simulationFrameWaiting = false;
    emitChanged();

    // Stop if requested or simulation error occurred
    if(_simulationStopping || result != StepCore::Solver::OK) {
        _undoStack->push(_simulationCommand);
        _undoStack->endMacro();
        _simulationCommand = NULL;

        _simulationTimer->stop();
        emit simulationStopped(result);
        return;
    }

    if(_simulationFrameSkipped) {
        _simulationFrameSkipped = false;
        QApplication::processEvents(); // XXX
        if(!_simulationFrameWaiting)
            simulationFrameBegin();
    }
}

