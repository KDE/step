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

#include "propertiesbrowser.h"
#include "propertiesbrowser.moc"

#include "worldmodel.h"
#include "worldfactory.h"
#include <stepcore/world.h>
#include <QAbstractItemModel>
#include <QTreeView>
#include <QMetaProperty>
#include <KLocale>

class PropertiesBrowserModel: public QAbstractItemModel
{
public:
    PropertiesBrowserModel(WorldModel* worldModel, QObject* parent = 0);

    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    void setObject(StepCore::Object* object) { _object = object; reset(); }
    StepCore::Object* object() { return _object; }

    void emitDataChanged() { emit dataChanged(createIndex(0,1), createIndex(rowCount()-1,1)); }

protected:
    WorldModel* _worldModel;
    StepCore::Object* _object;
};

PropertiesBrowserModel::PropertiesBrowserModel(WorldModel* worldModel, QObject* parent)
    : QAbstractItemModel(parent), _worldModel(worldModel), _object(NULL)
{
}

QVariant PropertiesBrowserModel::data(const QModelIndex &index, int role) const
{
    if(_object == NULL) return QVariant();

    if(!index.isValid()) return QVariant();

    const StepCore::MetaProperty* p = _object->metaObject()->property(index.row());
    if(role == Qt::DisplayRole || role == Qt::EditRole) {
        if(index.column() == 0) return p->name();
        else if(index.column() == 1) {
            /*if(p->userTypeId() < (int) QVariant::UserType) return p->readVariant(_object);
            else*/ return p->readString(_object);
        }
    } else if(role == Qt::ForegroundRole && index.column() == 1) {
        if(!p->isWritable()) return QBrush(Qt::darkGray); // XXX: how to get scheme color ?
    } else if(role == Qt::ToolTipRole) {
        return p->description(); // XXX: translation
    }

    return QVariant();
}

QModelIndex PropertiesBrowserModel::index(int row, int column, const QModelIndex &parent) const
{
    if(_object == NULL) return QModelIndex();
    if(!parent.isValid()) return createIndex(row, column);
    else return QModelIndex();
}

QModelIndex PropertiesBrowserModel::parent(const QModelIndex& /*index*/) const
{
    return QModelIndex();
}

int PropertiesBrowserModel::rowCount(const QModelIndex &parent) const
{
    if(_object == NULL) return 0;
    else if(parent.isValid()) return 0;
    else return _object->metaObject()->propertyCount();
}

int PropertiesBrowserModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 2;
}

QVariant PropertiesBrowserModel::headerData(int section, Qt::Orientation /*orientation*/,
                                      int role) const
{
    if (role != Qt::DisplayRole) return QVariant();
    switch(section) {
        case 0: return i18n("Property");
        case 1: return i18n("Value");
        default: return QVariant();
    }
}

Qt::ItemFlags PropertiesBrowserModel::flags(const QModelIndex &index) const
{
    if(_object == NULL) QAbstractItemModel::flags(index);

    if(index.isValid() && index.column() == 1 && _object->metaObject()->property(index.row())->isWritable())
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    else return QAbstractItemModel::flags(index);

}

bool PropertiesBrowserModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(_object == NULL) return false;

    if(index.isValid() && index.column() == 1 && role == Qt::EditRole) {
        const StepCore::MetaProperty* p = _object->metaObject()->property(index.row());
        /*if(p->userTypeId() < (int) QVariant::UserType) p->writeVariant(_object, value);
        else */p->writeString(_object, value.toString());
        emit dataChanged(index, index);
        _worldModel->setData(_worldModel->objectIndex(_object),
                                QVariant(), WorldModel::ObjectRole);
    }
    return false;
}

PropertiesBrowser::PropertiesBrowser(WorldModel* worldModel, QWidget* parent, Qt::WindowFlags flags)
    : QDockWidget("Properties", parent, flags)
{
    _worldModel = worldModel;
    _propertiesBrowserModel = new PropertiesBrowserModel(worldModel, this);
    _treeView = new QTreeView(this);
    _treeView->setModel(_propertiesBrowserModel);

    worldCurrentChanged(_worldModel->worldIndex(), QModelIndex());

    QObject::connect(_worldModel, SIGNAL(modelReset()), this, SLOT(worldModelReset()));
    QObject::connect(_worldModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                         this, SLOT(worldDataChanged(const QModelIndex&, const QModelIndex&)));
    QObject::connect(_worldModel->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                                           this, SLOT(worldCurrentChanged(const QModelIndex&, const QModelIndex&)));

    setWidget(_treeView);
}

void PropertiesBrowser::worldModelReset()
{
    _propertiesBrowserModel->setObject(NULL);
}

void PropertiesBrowser::worldCurrentChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
    _propertiesBrowserModel->setObject(_worldModel->object(current));
}

void PropertiesBrowser::worldDataChanged(const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/)
{
    _propertiesBrowserModel->emitDataChanged();
}


