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

#include "settings.h"

#include "worldfactory.h"
#include "unitscalc.h"

#include "worldmodel.h"
#include <stepcore/object.h>
#include <stepcore/solver.h>
#include <stepcore/types.h>
#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QItemEditorFactory>
#include <QTreeView>
#include <QMouseEvent>
#include <KLocale>
#include <KComboBox>

class ChoicesModel: public QStandardItemModel
{
public:
    ChoicesModel(QObject* parent = 0): QStandardItemModel(parent) {}
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

    void setObject(StepCore::Object* object);
    StepCore::Object* object() { return _object; }

    void emitDataChanged(bool dynamicOnly);

protected:
    WorldModel* _worldModel;
    StepCore::Object* _object;
    ChoicesModel* _solverChoices;
    QList<int> _subRows;
};

PropertiesBrowserModel::PropertiesBrowserModel(WorldModel* worldModel, QObject* parent)
    : QAbstractItemModel(parent), _worldModel(worldModel), _object(NULL)
{
    _solverChoices = new ChoicesModel(this);

    // Prepare solver list
    foreach(QString name, _worldModel->worldFactory()->orderedMetaObjects()) {
        const StepCore::MetaObject* metaObject = _worldModel->worldFactory()->metaObject(name);
        if(metaObject->isAbstract()) continue;
        if(!metaObject->inherits(StepCore::Solver::staticMetaObject())) continue;
        QString solverName = QString(metaObject->className()).replace("Solver", "");
        QStandardItem* item = new QStandardItem(solverName);
        item->setToolTip(QString(metaObject->description()));
        _solverChoices->appendRow(item);
    }
}

void PropertiesBrowserModel::setObject(StepCore::Object* object)
{
    _object = object;

    _subRows.clear();
    if(_object != NULL) {
        _worldModel->simulationPause();
        for(int i=0; i<_object->metaObject()->propertyCount(); ++i) {
            const StepCore::MetaProperty* p = _object->metaObject()->property(i);
            if(p->userTypeId() == qMetaTypeId<std::vector<StepCore::Vector2d> >())
                _subRows << p->readVariant(_object).value<std::vector<StepCore::Vector2d> >().size();
            else _subRows << 0;
        }
    }

    reset();
}

void PropertiesBrowserModel::emitDataChanged(bool dynamicOnly)
{
    if(_object == NULL) return;

    _worldModel->simulationPause();
    for(int i=0; i<_object->metaObject()->propertyCount(); i++) {
        const StepCore::MetaProperty* p = _object->metaObject()->property(i);
        if(dynamicOnly && !p->isDynamic()) continue;
        if(p->userTypeId() == qMetaTypeId<std::vector<StepCore::Vector2d> >()) {
            int r = p->readVariant(_object).value<std::vector<StepCore::Vector2d> >().size();
            if(r > _subRows[i]) {
                beginInsertRows(index(i, 0), _subRows[i], r-1);
                _subRows[i] = r;
                endInsertRows();
            } else if(r < _subRows[i]) {
                beginRemoveRows(index(i, 0), r, _subRows[i]-1);
                _subRows[i] = r;
                endRemoveRows();
            }
            if(r != 0) emit dataChanged(index(0,0,index(i,0)), index(r-1,1,index(i,0))); // XXX?
        }
        emit dataChanged(index(i,1), index(i,1));
    }
    //emit dataChanged(index(0,1), index(rowCount()-1,1));
}

QVariant PropertiesBrowserModel::data(const QModelIndex &index, int role) const
{
    if(_object == NULL) return QVariant();

    if(!index.isValid()) return QVariant();

    if(index.internalId() == 0) {
        const StepCore::MetaProperty* p = _object->metaObject()->property(index.row());
        if(role == Qt::DisplayRole || role == Qt::EditRole) {
            if(index.column() == 0) return p->name();
            else if(index.column() == 1) {
                _worldModel->simulationPause();

                // Solver type combobox ?
                if(index.row() == 1 && dynamic_cast<StepCore::Solver*>(_object)) {
                    if(role == Qt::DisplayRole) return p->readString(_object).replace("Solver", "");
                    else return QVariant::fromValue(_solverChoices);
                }

                int pr = Settings::floatDisplayPrecision();

                QString units;
                if(role == Qt::DisplayRole && !p->units().isEmpty()) 
                    units.append(" [").append(p->units()).append("]");
#ifdef STEP_WITH_UNITSCALC
                else if(role == Qt::EditRole && !p->units().isEmpty()
                            && p->userTypeId() == QMetaType::Double) 
                    units.append(" ").append(p->units());
#endif

                // Common property types
                if(p->userTypeId() == QMetaType::Double) {
                    return QString::number(p->readVariant(_object).toDouble(), 'g', pr).append(units);
                } else if(p->userTypeId() == qMetaTypeId<StepCore::Vector2d>()) {
                    StepCore::Vector2d v = p->readVariant(_object).value<StepCore::Vector2d>();
                    return QString("(%1,%2)%3").arg(v[0], 0, 'g', pr).arg(v[1], 0, 'g', pr).arg(units);
                } else if(p->userTypeId() == qMetaTypeId<std::vector<StepCore::Vector2d> >() ) {
                    std::vector<StepCore::Vector2d> list =
                            p->readVariant(_object).value<std::vector<StepCore::Vector2d> >();
                    QString string;
                    unsigned int end = qMax(10u, list.size()); // XXX: make it 
                    for(unsigned int i=0; i<end; ++i) {
                        if(!string.isEmpty()) string += ",";
                        string += QString("(%1,%2)%3").arg(list[i][0], 0, 'g', pr)
                                                        .arg(list[i][1], 0, 'g', pr);
                    }
                    if(role == Qt::DisplayRole && end != 0 && end < list.size()) string += ",...";
                    string.append(units);
                    return string;
                } else {
                    // default type
                    return p->readString(_object);
                }
                ///*if(p->userTypeId() < (int) QVariant::UserType) return p->readVariant(_object);
                //else*/ return p->readString(_object); // XXX: default delegate for double looks ugly!
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
    } else { // index.internalId() != 0
        const StepCore::MetaProperty* p = _object->metaObject()->property(index.internalId()-1);
        if(role == Qt::DisplayRole || role == Qt::EditRole) {
            if(index.column() == 0) return QString("%1[%2]").arg(p->name()).arg(index.row());
            else if(index.column() == 1) {
                QString units;
                if(role == Qt::DisplayRole && !p->units().isEmpty())
                    units.append(" [").append(p->units()).append("]");
#ifdef STEP_WITH_UNITSCALC
//                else if(role == Qt::EditRole && !p->units().isEmpty()) 
//                    units.append(" ").append(p->units());
#endif
                int pr = Settings::floatDisplayPrecision();
                _worldModel->simulationPause();
                StepCore::Vector2d v =
                        p->readVariant(_object).value<std::vector<StepCore::Vector2d> >()[index.row()];
                return QString("(%1,%2)%3").arg(v[0], 0, 'g', pr).arg(v[1], 0, 'g', pr).arg(units);
            }
        } else if(role == Qt::ForegroundRole && index.column() == 1) {
            if(!p->isWritable()) {
                return QBrush(Qt::darkGray); // XXX: how to get scheme color ?
            }
        } else if(role == Qt::ToolTipRole) {
            return p->description(); // XXX: translation
        }
    }

    return QVariant();
}

bool PropertiesBrowserModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(_object == NULL) return false;

    if(index.isValid() && index.column() == 1 && role == Qt::EditRole) {
        _worldModel->simulationPause();
        if(index.internalId() == 0) {
            if(index.row() == 0) { // name // XXX: do it more generally
                if(!_worldModel->checkUniqueName(value.toString())) return false; // XXX: error message
            }
            if(index.row() == 1 && dynamic_cast<StepCore::Solver*>(_object)) {
                if(value.toString() != _object->metaObject()->className()) {
                    _worldModel->beginMacro(i18n("Change solver type"));
                    _object = _worldModel->newSolver(value.toString() + "Solver");
                    Q_ASSERT(_object != NULL);
                    _worldModel->endMacro();
                    reset();
                }
            } else {
                const StepCore::MetaProperty* p = _object->metaObject()->property(index.row());
                QVariant v = value;

#ifdef STEP_WITH_UNITSCALC
                if(p->userTypeId() == QMetaType::Double) {
                    double number;
                    UnitsCalc::self()->parseNumber(value.toString(), p->units(), number);
                    v = number;
                }
#endif

                _worldModel->beginMacro(i18n("Edit %1", _object->name()));
                _worldModel->setProperty(_object, _object->metaObject()->property(index.row()), v);
                _worldModel->endMacro();
            }
        } else {
            const StepCore::MetaProperty* p = _object->metaObject()->property(index.internalId()-1);
            std::vector<StepCore::Vector2d> v =
                        p->readVariant(_object).value<std::vector<StepCore::Vector2d> >();
            bool ok;
            v[index.row()] = StepCore::stringToType<StepCore::Vector2d>(value.toString(), &ok);
            if(!ok) return true; // dataChanged should be emitted anyway
            _worldModel->beginMacro(i18n("Edit %1", _object->name()));
            _worldModel->setProperty(_object, p, QVariant::fromValue(v));
            _worldModel->endMacro();
        }
        return true;
    }
    return false;
}

QModelIndex PropertiesBrowserModel::index(int row, int column, const QModelIndex &parent) const
{
    if(_object == NULL) return QModelIndex();
    if(!parent.isValid()) return createIndex(row, column);

    if(parent.internalId() == 0 && _subRows[parent.row()] != 0)
        return createIndex(row, column, parent.row()+1);

    return QModelIndex();
}

QModelIndex PropertiesBrowserModel::parent(const QModelIndex& index) const
{
    if(index.isValid() && index.internalId() != 0)
        return createIndex(index.internalId()-1, 0, 0);
    return QModelIndex();
}

int PropertiesBrowserModel::rowCount(const QModelIndex &parent) const
{
    if(_object == NULL) return 0;
    else if(parent.isValid()) {
        if(parent.column() == 0 && parent.internalId() == 0) return _subRows[parent.row()];
        return 0;
    }
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
        if(index.internalId() == 0) {
            if(_object->metaObject()->property(index.row())->isWritable() ||
                (index.row()==1 && dynamic_cast<StepCore::Solver*>(_object))) flags |= Qt::ItemIsEditable;
        } else {
            if(_object->metaObject()->property(index.internalId()-1)->isWritable()) flags |= Qt::ItemIsEditable;
        }
    }

    return flags;
}

QWidget* PropertiesBrowserDelegate::createEditor(QWidget* parent,
                const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
    QVariant data = index.data(Qt::EditRole);
    int userType = data.userType();
    if(userType == qMetaTypeId<ChoicesModel*>()) {
        KComboBox* editor = new KComboBox(parent);
        editor->setModel(data.value<ChoicesModel*>());
        editor->installEventFilter(const_cast<PropertiesBrowserDelegate*>(this));
        connect(editor, SIGNAL(activated(int)), 
                this, SLOT(comboBoxActivated(int)));
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
    KComboBox* cb = qobject_cast<KComboBox*>(editor);
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
    KComboBox* cb = qobject_cast<KComboBox*>(editor);
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
    _treeView->setSelectionBehavior(QTreeView::SelectRows);
    _treeView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    //_treeView->setEditTriggers(/*QAbstractItemView::CurrentChanged | */QAbstractItemView::SelectedClicked |
    //                           QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
    _treeView->setItemDelegate(new PropertiesBrowserDelegate(_treeView));

    worldCurrentChanged(_worldModel->worldIndex(), QModelIndex());

    connect(_worldModel, SIGNAL(modelReset()), this, SLOT(worldModelReset()));
    connect(_worldModel, SIGNAL(worldDataChanged(bool)), this, SLOT(worldDataChanged(bool)));

    connect(_worldModel->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                                           this, SLOT(worldCurrentChanged(const QModelIndex&, const QModelIndex&)));

    connect(_treeView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                                           this, SLOT(currentChanged(const QModelIndex&, const QModelIndex&)));

    //connect(_treeView, SIGNAL(doubleClicked(const QModelIndex&)),
    //                                       this, SLOT(doubleClicked(const QModelIndex&)));

    connect(_propertiesBrowserModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                                           this, SLOT(rowsInserted(const QModelIndex&, int, int)));
    connect(_propertiesBrowserModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                                           this, SLOT(rowsRemoved(const QModelIndex&, int, int)));

    _treeView->viewport()->installEventFilter(this);
    //_treeView->setMouseTracking(true);

    setWidget(_treeView);
}

void PropertiesBrowser::worldModelReset()
{
    _propertiesBrowserModel->setObject(NULL);
}

void PropertiesBrowser::worldCurrentChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
    _propertiesBrowserModel->setObject(_worldModel->object(current));
    //_treeView->expandAll();
    for(int i=0; i<_propertiesBrowserModel->rowCount(); ++i) {
        QModelIndex index = _propertiesBrowserModel->index(i, 0);
        if(_propertiesBrowserModel->rowCount(index) <= 10) // XXX: make it configurable
            _treeView->setExpanded(index, true);
    }
}

void PropertiesBrowser::worldDataChanged(bool dynamicOnly)
{
    _propertiesBrowserModel->emitDataChanged(dynamicOnly);
}

void PropertiesBrowser::currentChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
    if(current.isValid() && current.column() == 0)
        _treeView->selectionModel()->setCurrentIndex(current.sibling(current.row(), 1), QItemSelectionModel::Current);
}

void PropertiesBrowser::rowsInserted(const QModelIndex& parent, int start, int end)
{
    int rowCount = _propertiesBrowserModel->rowCount(parent);
    if(rowCount > 10 && (rowCount - (start-end+1)) <= 10) {
        _treeView->setExpanded(parent, false);
    }
}

void PropertiesBrowser::rowsRemoved(const QModelIndex& parent, int start, int end)
{
    int rowCount = _propertiesBrowserModel->rowCount(parent);
    if(rowCount <= 10 && rowCount + (start-end+1) > 10) {
        _treeView->setExpanded(parent, true);
    }
}

/*
void PropertiesBrowser::doubleClicked(const QModelIndex& index)
{
    kDebug() << "doubleClicked" << endl;
    if(_propertiesBrowserModel->rowCount(index) > 0) {
        kDebug() << "   doubleClicked!!!" << endl;
        _treeView->setExpanded(index, !_treeView->isExpanded(index));
    }
}
*/

bool PropertiesBrowser::eventFilter(QObject* object, QEvent* event)
{
    if(object == _treeView->viewport() && event->type() == QEvent::MouseButtonDblClick) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QModelIndex index = _treeView->indexAt(mouseEvent->pos());
        if(_propertiesBrowserModel->rowCount(index) > 0)
            _treeView->setExpanded(index, !_treeView->isExpanded(index));
    }
    return false;
}

void PropertiesBrowser::settingsChanged()
{
    _propertiesBrowserModel->emitDataChanged(false);
}


