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
#include <KLocale>


WorldModel::WorldModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    _selectionModel = new QItemSelectionModel(this, this);
    _worldFactory = new WorldFactory();
    _world = new StepCore::World();
    resetWorld();
}

WorldModel::~WorldModel()
{
    delete _worldFactory;
}

void WorldModel::clearWorld()
{
    _world->clear();
    resetWorld();
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
    emit worldChanged(false);

    _selectionModel->setCurrentIndex(worldIndex(), QItemSelectionModel::SelectCurrent);
}

void WorldModel::emitChanged()
{
    emit dataChanged(worldIndex(), worldIndex());
    emit dataChanged(solverIndex(), solverIndex());
    if(itemCount() > 0) emit dataChanged(itemIndex(0), itemIndex(itemCount()-1));
    emit worldChanged(true);
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
    StepCore::Object* obj = NULL;
    if(index.isValid()) obj = static_cast<StepCore::Object*>(index.internalPointer());

    if(role == Qt::DisplayRole) {
        if(obj != NULL)
            return QString("%1: %2").arg(obj->name().isEmpty() ? i18n("<unnamed>") : obj->name())
                                       .arg(obj->metaObject()->className());

    } else if(role == WorldModel::ObjectRole) {
        return QVariant::fromValue<void*>(obj);

    }

    return QVariant();
}

bool WorldModel::setData(const QModelIndex& index, const QVariant& /*value*/, int role)
{
    if(!index.isValid()) return false;

    if(role == WorldModel::ObjectRole) {
        _world->doCalcFn();
        emitChanged();
        return true;
    }

    return false;
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
    addItem(item); return item;
}

void WorldModel::addItem(StepCore::Item* item)
{
    beginInsertRows(worldIndex(), itemCount(), itemCount());
    _world->addItem(item);
    _world->doCalcFn();
    endInsertRows();
    emitChanged();
}

void WorldModel::deleteItem(StepCore::Item* item)
{
    int itemIndex = _world->itemIndex(item);
    beginRemoveRows(worldIndex(), itemIndex, itemIndex);
    _world->deleteItem(item);
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

bool WorldModel::doWorldEvolve(double delta)
{
    bool ret = _world->doEvolve(delta);
    _world->doCalcFn();
    emitChanged();
    return ret;
}

bool WorldModel::saveXml(QIODevice* device)
{
    StepCore::XmlFile file(device);

    if(file.save(_world)) {
        emit worldChanged(false);
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

