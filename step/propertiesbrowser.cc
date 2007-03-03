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

#include "worldfactory.h"

#include "worldmodel.h"
#include <stepcore/object.h>
#include <stepcore/solver.h>
#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QItemEditorFactory>
#include <QComboBox>
#include <QTreeView>
#include <KLocale>

class ChoicesModel: public QStandardItemModel
{
public: ChoicesModel(QObject* parent = 0): QStandardItemModel(parent) {}
};
Q_DECLARE_METATYPE(ChoicesModel*)

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
    ChoicesModel* _solverChoices;
};

PropertiesBrowserModel::PropertiesBrowserModel(WorldModel* worldModel, QObject* parent)
    : QAbstractItemModel(parent), _worldModel(worldModel), _object(NULL)
{
    _solverChoices = new ChoicesModel(this);
    foreach(QString name, _worldModel->worldFactory()->orderedMetaObjects()) {
        const StepCore::MetaObject* metaObject = _worldModel->worldFactory()->metaObject(name);
        if(metaObject->isAbstract()) continue;
        if(!metaObject->inherits(StepCore::Solver::staticMetaObject())) continue;
        QStandardItem* item = new QStandardItem(QString(metaObject->className()));
        item->setToolTip(QString(metaObject->description()));
        _solverChoices->appendRow(item);
    }
}

QVariant PropertiesBrowserModel::data(const QModelIndex &index, int role) const
{
    if(_object == NULL) return QVariant();

    if(!index.isValid()) return QVariant();

    const StepCore::MetaProperty* p = _object->metaObject()->property(index.row());
    if(role == Qt::DisplayRole || role == Qt::EditRole) {
        if(index.column() == 0) return p->name();
        else if(index.column() == 1) {
            if(role == Qt::EditRole && index.row() == 1 && dynamic_cast<StepCore::Solver*>(_object)) {
                return QVariant::fromValue(_solverChoices);
            }
            /*if(p->userTypeId() < (int) QVariant::UserType) return p->readVariant(_object);
            else*/ return p->readString(_object); // XXX: default delegate for double looks ugly!
        }
    } else if(role == Qt::ForegroundRole && index.column() == 1) {
        if(!p->isWritable()) {
            if(index.row() != 1 || !dynamic_cast<StepCore::Solver*>(_object))
                return QBrush(Qt::darkGray); // XXX: how to get scheme color ?
        }
    } else if(role == Qt::ToolTipRole) {
        if(index.row() == 1 && index.column() == 1 && dynamic_cast<StepCore::Solver*>(_object)) {
            return _object->metaObject()->description();
        }
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
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if(_object && index.isValid() && index.column() == 1) {
        if(_object->metaObject()->property(index.row())->isWritable() ||
            (index.row()==1 && dynamic_cast<StepCore::Solver*>(_object))) flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool PropertiesBrowserModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(_object == NULL) return false;

    if(index.isValid() && index.column() == 1 && role == Qt::EditRole) {
        if(index.row() == 0) { // name // XXX: do it more generally
            if(!_worldModel->checkUniqueName(value.toString())) return false; // XXX: error message
        }
        if(index.row() == 1 && dynamic_cast<StepCore::Solver*>(_object)) {
            if(value.toString() != _object->metaObject()->className()) {
                _worldModel->beginMacro(i18n("Change solver type"));
                _object = _worldModel->newSolver(value.toString());
                Q_ASSERT(_object != NULL);
                _worldModel->endMacro();
                reset();
            }
        } else {
            _worldModel->beginMacro(i18n("Edit %1", _object->name()));
            _worldModel->setProperty(_object, _object->metaObject()->property(index.row()), value);
            _worldModel->endMacro();
        }
        return true;
    }
    return false;
}

QWidget* PropertiesBrowserDelegate::createEditor(QWidget* parent,
                const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
    QVariant data = index.data(Qt::EditRole);
    int userType = data.userType();
    if(userType == qMetaTypeId<ChoicesModel*>()) {
        QComboBox* editor = new QComboBox(parent);
        editor->setModel(data.value<ChoicesModel*>());
        editor->installEventFilter(const_cast<PropertiesBrowserDelegate*>(this));
        connect(editor, SIGNAL(activated(int)), this, SLOT(comboBoxActivated(int)));
        const_cast<PropertiesBrowserDelegate*>(this)->_editor = editor;
        return editor;
    } else {
        const QItemEditorFactory *factory = itemEditorFactory();
        if(!factory) factory = QItemEditorFactory::defaultFactory();
        return factory->createEditor(static_cast<QVariant::Type>(userType), parent);
    }
}

void PropertiesBrowserDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* cb = qobject_cast<QComboBox*>(editor);
    if(cb) {
        QVariant data = index.data(Qt::DisplayRole);
        ChoicesModel* cm = static_cast<ChoicesModel*>(cb->model());
        QList<QStandardItem*> items = cm->findItems(data.toString());
        Q_ASSERT(items.count() == 1);
        cb->setCurrentIndex( cm->indexFromItem(items[0]).row() );
    } else QItemDelegate::setEditorData(editor, index);
}

void PropertiesBrowserDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                   const QModelIndex& index) const
{
    QComboBox* cb = qobject_cast<QComboBox*>(editor);
    if(cb) {
        model->setData(index, cb->currentText());
    } else QItemDelegate::setModelData(editor, model, index);
}

void PropertiesBrowserDelegate::comboBoxActivated(int /*index*/)
{
    emit commitData(_editor);
}

PropertiesBrowser::PropertiesBrowser(WorldModel* worldModel, QWidget* parent, Qt::WindowFlags flags)
    : QDockWidget(i18n("Properties"), parent, flags)
{
    _worldModel = worldModel;
    _propertiesBrowserModel = new PropertiesBrowserModel(worldModel, this);
    _treeView = new QTreeView(this);
    _treeView->setModel(_propertiesBrowserModel);

    _treeView->setAllColumnsShowFocus(true);
    _treeView->setRootIsDecorated(false);
    //_treeView->setAlternatingRowColors(true);
    _treeView->setSelectionMode(QAbstractItemView::NoSelection);
    _treeView->setEditTriggers(QAbstractItemView::CurrentChanged);
    _treeView->setItemDelegate(new PropertiesBrowserDelegate(_treeView));

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


