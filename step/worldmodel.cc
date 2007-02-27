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
#include "worldmodel.moc"

#include "worldfactory.h"
#include <stepcore/world.h>
#include <stepcore/xmlfile.h>
#include <stepcore/eulersolver.h>
#include <QItemSelectionModel>
#include <QUndoStack>
#include <QTimer>
#include <KLocale>

class CommandEditProperty: public QUndoCommand
{
public:
    CommandEditProperty(WorldModel* worldModel, StepCore::Object* object,
                const StepCore::MetaProperty* property, const QVariant& newValue, bool merge = false);

    int id() const { return _merge ? 1 : -1; }
    bool mergeWith(const QUndoCommand* command);

    void redo();
    void undo();

protected:
    WorldModel* _worldModel;
    StepCore::Object* _object;
    const StepCore::MetaProperty* _property;
    QVariant _oldValue;
    QVariant _newValue;
    bool     _merge;
};

CommandEditProperty::CommandEditProperty(WorldModel* worldModel, StepCore::Object* object,
            const StepCore::MetaProperty* property, const QVariant& newValue, bool merge)
        : _worldModel(worldModel), _object(object), _property(property),
          _newValue(newValue), _merge(merge)
{
    _oldValue = _property->readVariant(_object);
    setText(i18n("edit %1").arg(object->name()));
    qDebug("edit %s, %s", object->name().toAscii().constData(), property->name());
}

void CommandEditProperty::redo()
{
    if(_newValue.type() != QVariant::String) _property->writeVariant(_object, _newValue);
    else _property->writeString(_object, _newValue.value<QString>());
    _worldModel->objectChanged(_object);
}

void CommandEditProperty::undo()
{
    _property->writeVariant(_object, _oldValue);
    _worldModel->objectChanged(_object);
}

bool CommandEditProperty::mergeWith(const QUndoCommand* command)
{
    const CommandEditProperty* cmd = dynamic_cast<const CommandEditProperty*>(command);
    Q_ASSERT(cmd != NULL);
    if(cmd->_object != _object || cmd->_property != _property) return false;

    _newValue = cmd->_newValue;
    return true;
}

class CommandNewItem: public QUndoCommand
{
public:
    CommandNewItem(WorldModel* worldModel, StepCore::Item* item, bool create);
    ~CommandNewItem();
    void redo();
    void undo();
protected:
    WorldModel* _worldModel;
    StepCore::Item* _item;
    bool _create;
    bool _shouldDelete;
};

CommandNewItem::CommandNewItem(WorldModel* worldModel, StepCore::Item* item, bool create)
    : _worldModel(worldModel), _item(item), _create(create), _shouldDelete(create)
{
    setText("TODO");
    qDebug("new %s", item->name().toAscii().constData());
}

CommandNewItem::~CommandNewItem()
{
    if(_shouldDelete) delete _item;
}

void CommandNewItem::redo()
{
    if(_create) _worldModel->addItem(_item);
    else _worldModel->removeItem(_item);
    _shouldDelete = !_create;
}

void CommandNewItem::undo()
{
    if(_create) _worldModel->removeItem(_item);
    else _worldModel->addItem(_item);
    _shouldDelete = _create;
}

class CommandSimulate: public QUndoCommand
{
public:
    CommandSimulate(WorldModel* worldModel);
    ~CommandSimulate() { delete _worldCopy; }
    void redo();
    void undo();

protected:
    WorldModel* _worldModel;
    StepCore::World* _worldCopy;
    StepCore::World* _oldWorld;
    StepCore::World* _newWorld;
};

CommandSimulate::CommandSimulate(WorldModel* worldModel)
{
    _worldModel = worldModel;
    _oldWorld = _worldCopy = _worldModel->_world;
    _newWorld = _worldModel->_world = new StepCore::World(*_oldWorld);
    _worldModel->reset();
    _worldModel->_selectionModel->setCurrentIndex( // XXX: preserve selection
            _worldModel->worldIndex(), QItemSelectionModel::SelectCurrent);
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
    QObject::connect(_simulationTimer, SIGNAL(timeout()),
                        this, SLOT(simulationFrame()));
    resetWorld();
}

WorldModel::~WorldModel()
{
    delete _worldFactory;
    delete _world;
}

void WorldModel::resetWorld()
{
    if(_world->name().isEmpty()) {
        // XXX: set default name for unnamed objects in setData !
        _world->setName(getUniqueName("World"));
    }
    if(NULL == _world->solver()) {
        _world->setSolver(new StepCore::EulerSolver());
        _world->solver()->setName(getUniqueName("EulerSolver"));
    }
    _world->doCalcFn();

    reset();
    _undoStack->clear();
    _selectionModel->setCurrentIndex(worldIndex(), QItemSelectionModel::SelectCurrent);
}

void WorldModel::emitChanged()
{
    emit dataChanged(worldIndex(), worldIndex());
    emit dataChanged(solverIndex(), solverIndex());
    if(itemCount() > 0) emit dataChanged(itemIndex(0), itemIndex(itemCount()-1));
}

QModelIndex WorldModel::worldIndex() const
{
    return createIndex(0, 0, _world);
}

QModelIndex WorldModel::solverIndex() const
{
    return createIndex(1, 0, _world->solver());
}

QModelIndex WorldModel::itemIndex(int n) const
{
    return createIndex(n, 0, _world->items()[n]);
}

QModelIndex WorldModel::objectIndex(StepCore::Object* obj) const
{
    if(obj == _world) return worldIndex();
    else if(obj == _world->solver()) return solverIndex();
    else return itemIndex(_world->itemIndex(dynamic_cast<const StepCore::Item*>(obj)));
}

StepCore::Object* WorldModel::object(const QModelIndex& index) const
{
    if(index.isValid()) return static_cast<StepCore::Object*>(index.internalPointer());
    else return NULL;
}

void WorldModel::objectChanged(const StepCore::Object* /*object*/)
{
    //if(index.isValid()) {
    _world->doCalcFn();
    emitChanged();
    //}
}

StepCore::Item* WorldModel::item(const QModelIndex& index) const
{
    return dynamic_cast<StepCore::Item*>(object(index));
}

int WorldModel::itemCount() const
{
    return _world->items().size();
}

QVariant WorldModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return QVariant();

    if(role == Qt::DisplayRole) {
        StepCore::Object* obj = static_cast<StepCore::Object*>(index.internalPointer());
        return QString("%1: %2").arg(obj->name().isEmpty() ? i18n("<unnamed>") : obj->name())
                                   .arg(obj->metaObject()->className());

    }

    return QVariant();
}

QModelIndex WorldModel::index(int row, int /*column*/, const QModelIndex &parent) const
{
    if(!parent.isValid()) {
        if(row == 0) return worldIndex();
        else if(row == 1) return solverIndex();
    } else if(parent.internalPointer() == _world) return itemIndex(row);
    return QModelIndex();
}

QModelIndex WorldModel::parent(const QModelIndex &index) const
{
    if(!index.isValid()) return QModelIndex();
    else if(index.internalPointer() == _world) return QModelIndex();
    else if(index.internalPointer() == _world->solver()) return QModelIndex();
    return worldIndex();
}

int WorldModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid()) return 2;
    else if(parent.internalPointer() == _world) return itemCount();
    else return 0;
}

int WorldModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 1;
}

StepCore::Item* WorldModel::newItem(const QString& name)
{
    StepCore::Item* item = _worldFactory->newItem(name);
    if(item == NULL) return NULL;
    item->setName(getUniqueName(name));
    pushCommand(new CommandNewItem(this, item, true));
    return item;
}

void WorldModel::deleteItem(StepCore::Item* item)
{
    pushCommand(new CommandNewItem(this, item, false));
}

void WorldModel::addItem(StepCore::Item* item)
{
    beginInsertRows(worldIndex(), itemCount(), itemCount());
    _world->addItem(item);
    _world->doCalcFn();
    endInsertRows();
    emitChanged();
}

void WorldModel::removeItem(StepCore::Item* item)
{
    int itemIndex = _world->itemIndex(item);
    beginRemoveRows(worldIndex(), itemIndex, itemIndex);
    _world->removeItem(item);
    _world->doCalcFn();
    endRemoveRows();
    emitChanged();
}

void WorldModel::setSolver(StepCore::Solver* solver)
{
    _world->setSolver(solver);
    _world->doCalcFn();
    emitChanged();
}

void WorldModel::pushCommand(QUndoCommand* command)
{
    if(!isSimulationActive()) {
        _undoStack->push(command);
    } else {
        command->redo();
        delete command;
    }
}

void WorldModel::beginMacro(const QString& text)
{
    if(!isSimulationActive())
        _undoStack->beginMacro(text);
}

void WorldModel::endMacro()
{
    if(!isSimulationActive())
        _undoStack->endMacro();
}

void WorldModel::setProperty(StepCore::Object* object, const StepCore::MetaProperty* property,
                                const QVariant& value, bool merge)
{
    pushCommand(new CommandEditProperty(this, object, property, value, merge));
}

bool WorldModel::doWorldEvolve(double delta)
{
    bool ret = _world->doEvolve(delta);
    _world->doCalcFn();
    emitChanged();
    return ret;
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

QString WorldModel::getUniqueName(QString className) const
{
    className[0] = className[0].toLower();
    for(int n=1; ; ++n) {
        QString name = className + QString::number(n);
        if(_world->name() == name) break;
        if(_world->solver() && _world->solver()->name() == name) break;
        StepCore::World::ItemList::const_iterator it = _world->items().begin();
        for(; it != _world->items().end(); ++it) {
            if((*it)->name() == name) break;
        }
        if(it == _world->items().end()) return name;
    }
    return QString();
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
    _undoStack->beginMacro("TODO");
    _simulationCommand = new CommandSimulate(this);
    _simulationTimer->start();
}

void WorldModel::simulationStop(bool success)
{
    _simulationTimer->stop();
    if(_simulationCommand) {
        _undoStack->push(_simulationCommand);
        _undoStack->endMacro();
        _simulationCommand = NULL;
    }
    emit simulationStopped(success);
}

void WorldModel::simulationFrame()
{
    bool ret = _world->doEvolve(1.0/_simulationFps);
    _world->doCalcFn();
    emitChanged();
    if(!ret) simulationStop(false);
}

