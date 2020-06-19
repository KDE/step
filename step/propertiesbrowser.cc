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

#include "settings.h"

#include "worldfactory.h"
#include "unitscalc.h"

#include "worldmodel.h"
#include <stepcore/object.h>
#include <stepcore/solver.h>
#include <stepcore/types.h>

#include <QAbstractItemModel>
#include <QApplication>
#include <QHBoxLayout>
#include <QItemEditorFactory>
#include <QMouseEvent>
#include <QTreeView>

#include <KColorButton>
#include <KComboBox>
#include <KLineEdit>
#include <KLocalizedString>

#include "choicesmodel.h"

class PropertiesBrowserModel: public QAbstractItemModel
{
public:
    PropertiesBrowserModel(WorldModel* worldModel, QObject* parent = 0);

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role) Q_DECL_OVERRIDE;

    void setObject(StepCore::Object* object);
    StepCore::Object* object() { return _object; }

    void emitDataChanged(bool dynamicOnly);

protected:
    WorldModel* _worldModel;
    StepCore::Object*       _object;
    StepCore::Item*         _item;
    StepCore::ObjectErrors* _objectErrors;
    ChoicesModel* _solverChoices;
    QList<int> _subRows;
};

PropertiesBrowserModel::PropertiesBrowserModel(WorldModel* worldModel, QObject* parent)
    : QAbstractItemModel(parent), _worldModel(worldModel), _object(NULL)
{
    _solverChoices = new ChoicesModel(this);

    // Prepare solver list
    foreach(const QString &name, _worldModel->worldFactory()->orderedMetaObjects()) {
        const StepCore::MetaObject* metaObject = _worldModel->worldFactory()->metaObject(name);
        if(metaObject->isAbstract()) continue;
        if(!metaObject->inherits(StepCore::Solver::staticMetaObject())) continue;
        QString solverName = QString(metaObject->className()).remove(QStringLiteral("Solver"));
        QStandardItem* item = new QStandardItem(solverName);
        item->setToolTip(QString(metaObject->descriptionTr()));
        _solverChoices->appendRow(item);
    }
}

void PropertiesBrowserModel::setObject(StepCore::Object* object)
{
    beginResetModel();
    _object = object;

    _subRows.clear();
    if(_object != NULL) {
        _worldModel->simulationPause();

        _item = dynamic_cast<StepCore::Item*>(_object);
        if(_item) {
            if(_item->world()->errorsCalculation()) _objectErrors = _item->objectErrors();
            else _objectErrors = _item->tryGetObjectErrors();
        } else {
            _objectErrors = NULL;
        }

        for(int i=0; i<_object->metaObject()->propertyCount(); ++i) {
            const StepCore::MetaProperty* p = _object->metaObject()->property(i);
            if(p->userTypeId() == qMetaTypeId<StepCore::Vector2dList >())
                _subRows << p->readVariant(_object).value<StepCore::Vector2dList >().size();
            else _subRows << 0;
        }
    }

    endResetModel();
}

void PropertiesBrowserModel::emitDataChanged(bool dynamicOnly)
{
    if(_object == NULL) return;

    _worldModel->simulationPause();

    _item = dynamic_cast<StepCore::Item*>(_object);
    if(_item) {
        if(_item->world()->errorsCalculation()) _objectErrors = _item->objectErrors();
        else _objectErrors = _item->tryGetObjectErrors();
    } else {
        _objectErrors = NULL;
    }

    for(int i=0; i<_object->metaObject()->propertyCount(); i++) {
        const StepCore::MetaProperty* p = _object->metaObject()->property(i);
        if(dynamicOnly && !p->isDynamic()) continue;
        if(p->userTypeId() == qMetaTypeId<StepCore::Vector2dList >()) {
            int r = p->readVariant(_object).value<StepCore::Vector2dList >().size();
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
            if(index.column() == 0) return p->nameTr();
            else if(index.column() == 1) {
                _worldModel->simulationPause();

                // Solver type combobox ?
                if(index.row() == 1 && dynamic_cast<StepCore::Solver*>(_object)) {
                    if(role == Qt::DisplayRole) return p->readString(_object).remove(QStringLiteral("Solver"));
                    else return QVariant::fromValue(_solverChoices);
                }

                if(p->userTypeId() == QMetaType::Double ||
                    p->userTypeId() == qMetaTypeId<StepCore::Vector2d>() ||
                    p->userTypeId() == qMetaTypeId<StepCore::Vector2dList >()) {
                    return _worldModel->formatProperty(_object, _objectErrors, p,
                                role == Qt::EditRole ? WorldModel::FormatEditable : WorldModel::FormatFlags(0));
                } else if(p->userTypeId() == qMetaTypeId<StepCore::Object*>()) {
                    return _worldModel->formatName(p->readVariant(_object).value<StepCore::Object*>());
                } else if(p->userTypeId() == qMetaTypeId<StepCore::Color>()) {
                    Q_ASSERT( !_objectErrors || !_objectErrors->metaObject()->property(p->name() + "Variance") );
                    Q_ASSERT( p->units().isEmpty() );
                    if(role == Qt::EditRole)
                        return QColor::fromRgba(p->readVariant(_object).value<StepCore::Color>());
                    else
                        return p->readString(_object);
                } else if(p->userTypeId() == QMetaType::Bool) {
                    Q_ASSERT( !_objectErrors || !_objectErrors->metaObject()->property(p->name() + "Variance") );
                    Q_ASSERT( p->units().isEmpty() );
                    return p->readVariant(_object);
                } else {
                    // default type
                    // XXX: add error information
                    //if(pe) error = QString::fromUtf8(" ± ").append(pe->readString(_objectErrors)).append(units);
                    //if(pv) qDebug() << "Unhandled property variance type" << endl;
                    Q_ASSERT( !_objectErrors || !_objectErrors->metaObject()->property(p->name() + "Variance") );
                    Q_ASSERT( p->units().isEmpty() );
                    if(role == Qt::EditRole) return _worldModel->formatProperty(_object, _objectErrors, p,
                                        WorldModel::FormatHideUnits | WorldModel::FormatEditable);
                    else return _worldModel->formatProperty(_object, _objectErrors, p, WorldModel::FormatHideUnits);
                }
                ///*if(p->userTypeId() < (int) QVariant::UserType) return p->readVariant(_object);
                //else*/ return p->readString(_object); // XXX: default delegate for double looks ugly!
            }
        } else if(index.column() == 1 && role == Qt::ForegroundRole) {
            if(!p->isWritable()) {
                if(index.row() != 1 || !dynamic_cast<StepCore::Solver*>(_object))
                    return QBrush(Qt::darkGray); // XXX: how to get scheme color ?
            }
        } else if(role == Qt::ToolTipRole) {
            if(index.row() == 1 && index.column() == 1 && dynamic_cast<StepCore::Solver*>(_object)) {
                return _object->metaObject()->descriptionTr();
            }
            return p->descriptionTr(); // XXX: translation
        } else if(index.column() == 1 && role == Qt::DecorationRole &&
                    p->userTypeId() == qMetaTypeId<StepCore::Color>()) {
            QPixmap pix(8, 8);
            pix.fill(QColor::fromRgba(p->readVariant(_object).value<StepCore::Color>()));
            return pix;
        } else if(index.column() == 0 && role == Qt::DecorationRole &&
                    p->userTypeId() == qMetaTypeId<StepCore::Vector2dList >() &&
                    rowCount(index) > 0) {
            // XXX: A hack to have nested properties shifted
            static QPixmap empySmallPix;
            if(empySmallPix.isNull()) {
                empySmallPix = QPixmap(8,8); //XXX
                empySmallPix.fill(QColor(0,0,0,0));
            }
            return empySmallPix;
        }
    } else { // index.internalId() != 0
        const StepCore::MetaProperty* p = _object->metaObject()->property(index.internalId()-1);
        if(role == Qt::DisplayRole || role == Qt::EditRole) {
            if(index.column() == 0) return QStringLiteral("%1[%2]").arg(p->nameTr()).arg(index.row());
            else if(index.column() == 1) {
#ifdef __GNUC__
#warning XXX: add error information for lists
#endif
                QString units;
                if(role == Qt::DisplayRole && !p->units().isEmpty())
                    units.append(" [").append(p->units()).append("]");
#ifdef STEP_WITH_UNITSCALC
//                else if(role == Qt::EditRole && !p->units().isEmpty()) 
//                    units.append(" ").append(p->units());
#endif
                int pr = Settings::floatDisplayPrecision();
                //int pr = role == Qt::DisplayRole ? Settings::floatDisplayPrecision() : 16;
                _worldModel->simulationPause();
                StepCore::Vector2d v =
                        p->readVariant(_object).value<StepCore::Vector2dList >()[index.row()];
                return QStringLiteral("(%1,%2)%3").arg(v[0], 0, 'g', pr).arg(v[1], 0, 'g', pr).arg(units);
            }
        } else if(role == Qt::ForegroundRole && index.column() == 1) {
            if(!p->isWritable()) {
                return QBrush(Qt::darkGray); // XXX: how to get scheme color ?
            }
        } else if(role == Qt::ToolTipRole) {
            return p->descriptionTr(); // XXX: translation
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
                    beginResetModel();
                    _worldModel->beginMacro(i18n("Change solver type"));
                    _object = _worldModel->newSolver(value.toString() + "Solver");
                    Q_ASSERT(_object != NULL);
                    _worldModel->endMacro();
                    endResetModel();
                }
            } else {
                const StepCore::MetaProperty* p = _object->metaObject()->property(index.row());
                const StepCore::MetaProperty* pv = _objectErrors ?
                        _objectErrors->metaObject()->property(p->name() + "Variance") : NULL;

                if(p->userTypeId() == qMetaTypeId<StepCore::Object*>()) {
                    Q_ASSERT(!pv);
                    StepCore::Object* obj = _worldModel->world()->object(value.toString());
                    if(!obj) return false;
                    _worldModel->beginMacro(i18n("Change %1.%2", _object->name(), p->nameTr()));
                    _worldModel->setProperty(_object, p, QVariant::fromValue(obj));
                    _worldModel->endMacro();
                    return true;
                } else if(p->userTypeId() == qMetaTypeId<StepCore::Color>()) {
                    Q_ASSERT(!pv);
                    _worldModel->beginMacro(i18n("Change %1.%2", _object->name(), p->nameTr()));
                    _worldModel->setProperty(_object, p, value.type() == QVariant::String ? value :
                                    QVariant::fromValue(StepCore::Color(value.value<QColor>().rgba())));
                    _worldModel->endMacro();
                    return true;
                } else if(p->userTypeId() == qMetaTypeId<bool>()) {
                    Q_ASSERT(!pv);
                    _worldModel->beginMacro(i18n("Change %1.%2", _object->name(), p->nameTr()));
                    _worldModel->setProperty(_object, p, value);
                    _worldModel->endMacro();
                    return true;
                } else if(p->userTypeId() == qMetaTypeId<QString>()) {
                    Q_ASSERT(!pv);
                    if(index.row() == 0)
                        _worldModel->beginMacro(i18n("Rename %1 to %2", _object->name(), value.toString()));
                    else
                        _worldModel->beginMacro(i18n("Change %1.%2", _object->name(), p->nameTr()));
                    _worldModel->setProperty(_object, p, value);
                    _worldModel->endMacro();
                    return true;
                }

                QVariant v = value;
                QVariant vv;

                // Try to find ± sign
                if(v.canConvert(QVariant::String)) {
                    QString str = v.toString();
                    int idx = str.indexOf(QStringLiteral("±"));
                    if(idx >= 0) {
                        v = str.left(idx);
                        vv = str.mid(idx+1);
                    }
                }

#ifdef STEP_WITH_UNITSCALC
                    // Convert units
                    if(p->userTypeId() == QMetaType::Double) {
                        double number = 0;
                        if(UnitsCalc::self()->parseNumber(v.toString(), p->units(), number)) {
                            v = number;
                        } else {
                            return false;
                        }
                        if(vv.isValid()) {
                            if(UnitsCalc::self()->parseNumber(vv.toString(), p->units(), number)) {
                                vv = number;
                            } else {
                                return false;
                            }
                        }
                    }
#endif

                if(vv.isValid()) { // We have got variance value
                    if(!pv) {
                        // check if _objectErrors can be created
                        // and current property variance could be set
                        const StepCore::MetaObject* me =
                            _worldModel->worldFactory()->metaObject(
                                _object->metaObject()->className() + "Errors");
                        if(!_item || !me || !me->property(p->name() + "Variance"))
                            return false;
                    }

                    bool ok = true;
                    // Calc variance = square(error)
                    if(p->userTypeId() == QMetaType::Double) {
                        vv = StepCore::square(vv.toDouble(&ok));
                    } else if(p->userTypeId() == qMetaTypeId<StepCore::Vector2d>()) {
                        StepCore::Vector2d svv;
                        svv = StepCore::stringToType<StepCore::Vector2d>(vv.toString(), &ok);
                        svv[0] *= svv[0]; svv[1] *= svv[1];
                        vv = QVariant::fromValue(svv);
                    /* XXX
                     * {} else if(p->userTypeId() == qMetaTypeId<StepCore::Vector2dList >())
                        ve = QVariant::fromValue(StepCore::Vector2dList());*/
                    } else {
//                         qDebug() << "Unhandled property variance type" << endl;
                        return false;
                    }
                    if(!ok) return false;

                } else { // vv.isValid()
                    if(pv) { // We have to zero variance since we got exact value
                        if(p->userTypeId() == QMetaType::Double) {
                            vv = 0;
                        } else if(p->userTypeId() == qMetaTypeId<StepCore::Vector2d>()) {
                            StepCore::Vector2d svv = StepCore::Vector2d::Zero();
                            vv = QVariant::fromValue(svv);
                        /* XXX
                         * } else if(p->userTypeId() == qMetaTypeId<StepCore::Vector2dList >())
                            ve = QVariant::fromValue(StepCore::Vector2dList());*/
                        } else {
                            qWarning("Unhandled property variance type");
                            return false;
                        }
                    }
                }

                _worldModel->beginMacro(i18n("Change %1.%2", _object->name(), p->nameTr()));
                _worldModel->setProperty(_object, p, v);
                if(vv.isValid() && !pv) {
                    // XXX: Make this undo-able
                    _objectErrors = _item->objectErrors();
                    pv = _objectErrors->metaObject()->property(p->name() + "Variance");
                }
                if(pv) _worldModel->setProperty(_objectErrors, pv, vv);
                _worldModel->endMacro();
            }
        } else {
            const StepCore::MetaProperty* p = _object->metaObject()->property(index.internalId()-1);
            StepCore::Vector2dList v =
                        p->readVariant(_object).value<StepCore::Vector2dList >();
            bool ok;
            v[index.row()] = StepCore::stringToType<StepCore::Vector2d>(value.toString(), &ok);
            if(!ok) return true; // dataChanged should be emitted anyway
            _worldModel->beginMacro(i18n("Change %1.%2", _object->name(), p->nameTr()));
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
        return createIndex(index.internalId()-1, 0, nullptr);
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
        connect(editor, SIGNAL(activated(int)), this, SLOT(editorActivated()));
        editor->installEventFilter(const_cast<PropertiesBrowserDelegate*>(this));
        const_cast<PropertiesBrowserDelegate*>(this)->_editor = editor;
        const_cast<PropertiesBrowserDelegate*>(this)->_comboBox = editor;
        const_cast<PropertiesBrowserDelegate*>(this)->_editorType = SolverChoiser;
        return editor;

    } else if(userType == QMetaType::QColor) {
        QWidget* editor = new QWidget(parent);

        KLineEdit* lineEdit = new KLineEdit(editor);
        lineEdit->setFrame(false);

        KColorButton* colorButton = new KColorButton(editor);
        // XXX: do not use hard-coded pixel sizes
        colorButton->setMinimumWidth(15);
        colorButton->setMaximumWidth(15);
        connect(colorButton, &KColorButton::changed, this, &PropertiesBrowserDelegate::editorActivated);

        QHBoxLayout* layout = new QHBoxLayout(editor);
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);
        layout->addWidget(lineEdit);
        layout->addWidget(colorButton);

        editor->setFocusProxy(lineEdit);
        editor->installEventFilter(const_cast<PropertiesBrowserDelegate*>(this));

        const_cast<PropertiesBrowserDelegate*>(this)->_editor = editor;
        const_cast<PropertiesBrowserDelegate*>(this)->_colorButton = colorButton;
        const_cast<PropertiesBrowserDelegate*>(this)->_lineEdit = lineEdit;
        const_cast<PropertiesBrowserDelegate*>(this)->_editorType = ColorChoiser;
        return editor;

    } else if(userType == QMetaType::Bool) {
        KComboBox* editor = new KComboBox(parent);
        editor->addItem(i18n("false"));
        editor->addItem(i18n("true"));
        connect(editor, SIGNAL(activated(int)), this, SLOT(editorActivated()));
        editor->installEventFilter(const_cast<PropertiesBrowserDelegate*>(this));
        const_cast<PropertiesBrowserDelegate*>(this)->_editor = editor;
        const_cast<PropertiesBrowserDelegate*>(this)->_comboBox = editor;
        const_cast<PropertiesBrowserDelegate*>(this)->_editorType = BoolChoiser;
        return editor;

    } else {
        const_cast<PropertiesBrowserDelegate*>(this)->_editorType = Standard;
        const QItemEditorFactory *factory = itemEditorFactory();
        if(!factory) factory = QItemEditorFactory::defaultFactory();
        return factory->createEditor(static_cast<QVariant::Type>(userType), parent);
    }
}

void PropertiesBrowserDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if(_editorType == SolverChoiser) {
        QVariant data = index.data(Qt::DisplayRole);
        ChoicesModel* cm = static_cast<ChoicesModel*>(_comboBox->model());
        QList<QStandardItem*> items = cm->findItems(data.toString());
        Q_ASSERT(items.count() == 1);
        _comboBox->setCurrentIndex( cm->indexFromItem(items[0]).row() );
    } else if(_editorType == ColorChoiser) {
        QVariant data = index.data(Qt::EditRole);
        QVariant data1 = index.data(Qt::DisplayRole);
        _updating = true;
        _colorButton->setColor(data.value<QColor>());
        _lineEdit->setText(data1.toString());
        _updating = false;
    } else if(_editorType == BoolChoiser) {
        bool value = index.data(Qt::EditRole).toBool();
        _comboBox->setCurrentIndex(value ? 1 : 0);
    } else QItemDelegate::setEditorData(editor, index);
}

void PropertiesBrowserDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                   const QModelIndex& index) const
{
    if(_editorType == SolverChoiser) {
        model->setData(index, _comboBox->currentText());
    } else if(_editorType == ColorChoiser) {
        model->setData(index, _lineEdit->text());
    } else if(_editorType == BoolChoiser) {
        model->setData(index, _comboBox->currentIndex());
    } else QItemDelegate::setModelData(editor, model, index);
}

void PropertiesBrowserDelegate::editorActivated()
{
    if(!_updating) {
        if(_editorType == ColorChoiser) {
            QRgb v = _colorButton->color().rgba();
            _lineEdit->setText(StepCore::typeToString<StepCore::Color>(v));
        }
        emit commitData(_editor);
        emit closeEditor(_editor);
    }
}

class PropertiesBrowserView: public QTreeView
{
public:
    PropertiesBrowserView(QWidget* parent = 0);
protected:
    void changeEvent(QEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QStyleOptionViewItem viewOptions() const Q_DECL_OVERRIDE;
    const int _windowsDecoSize;
    bool _macStyle;
};

PropertiesBrowserView::PropertiesBrowserView(QWidget* parent)
        : QTreeView(parent), _windowsDecoSize(9)
{
    _macStyle = QApplication::style()->inherits("QMacStyle");
}

void PropertiesBrowserView::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::StyleChange)
        _macStyle = QApplication::style()->inherits("QMacStyle");
}

void PropertiesBrowserView::mousePressEvent(QMouseEvent* event)
{
    if(columnAt(event->x()) == 0) {
        QModelIndex idx = indexAt(event->pos());
        if(idx.isValid() && !idx.parent().isValid() && idx.model()->rowCount(idx) > 0) {
            QRect primitive = visualRect(idx); primitive.setWidth(indentation());
            if (!_macStyle) {
                primitive.moveLeft(primitive.left() + (primitive.width() - _windowsDecoSize)/2);
                primitive.moveTop(primitive.top() + (primitive.height() - _windowsDecoSize)/2);
                primitive.setWidth(_windowsDecoSize);
                primitive.setHeight(_windowsDecoSize);
            }
            if(primitive.contains(event->pos())) {
                setExpanded(idx, !isExpanded(idx));
                
                return;
            }
        }
    }
    QTreeView::mousePressEvent(event);
}

void PropertiesBrowserView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    // Inspired by qt-designer code in src/components/propertyeditor/qpropertyeditor.cpp
    QStyleOptionViewItem opt = viewOptions();

    if(model()->hasChildren(index)) {
        opt.state |= QStyle::State_Children;

        QRect primitive(rect.left() + rect.width() - indentation(), rect.top(),
                                                    indentation(), rect.height());
        if(!index.parent().isValid()) {
            primitive.moveLeft(0);
        }

        if (!_macStyle) {
            primitive.moveLeft(primitive.left() + (primitive.width() - _windowsDecoSize)/2);
            primitive.moveTop(primitive.top() + (primitive.height() - _windowsDecoSize)/2);
            primitive.setWidth(_windowsDecoSize);
            primitive.setHeight(_windowsDecoSize);
        }

        opt.rect = primitive;

        if(isExpanded(index)) opt.state |= QStyle::State_Open;
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
    }
}

QStyleOptionViewItem PropertiesBrowserView::viewOptions() const
{
    QStyleOptionViewItem option = QTreeView::viewOptions();
    option.showDecorationSelected = true;
    return option;
}

PropertiesBrowser::PropertiesBrowser(WorldModel* worldModel, QWidget* parent)
    : QDockWidget(i18n("Properties"), parent)
{
    _worldModel = worldModel;
    _propertiesBrowserModel = new PropertiesBrowserModel(worldModel, this);
    _treeView = new PropertiesBrowserView(this);

    _treeView->setAllColumnsShowFocus(true);
    _treeView->setRootIsDecorated(false);
    //_treeView->setAlternatingRowColors(true);
    _treeView->setSelectionMode(QAbstractItemView::NoSelection);
    _treeView->setSelectionBehavior(QTreeView::SelectRows);
    _treeView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    //_treeView->setEditTriggers(/*QAbstractItemView::CurrentChanged | */QAbstractItemView::SelectedClicked |
    //                           QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
    _treeView->setItemDelegate(new PropertiesBrowserDelegate(_treeView));

    _treeView->setModel(_propertiesBrowserModel);
    worldCurrentChanged(_worldModel->worldIndex(), QModelIndex());

    connect(_worldModel, &QAbstractItemModel::modelReset, this, &PropertiesBrowser::worldModelReset);
    connect(_worldModel, &WorldModel::worldDataChanged, this, &PropertiesBrowser::worldDataChanged);
    connect(_worldModel, &QAbstractItemModel::rowsRemoved,
                                this, &PropertiesBrowser::worldRowsRemoved);

    connect(_worldModel->selectionModel(), &QItemSelectionModel::currentChanged,
                                           this, &PropertiesBrowser::worldCurrentChanged);

    connect(_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
                                           this, &PropertiesBrowser::currentChanged);

    //connect(_treeView, SIGNAL(doubleClicked(QModelIndex)),
    //                                       this, SLOT(doubleClicked(QModelIndex)));

    connect(_propertiesBrowserModel, &QAbstractItemModel::rowsInserted,
                                           this, &PropertiesBrowser::rowsInserted);
    connect(_propertiesBrowserModel, &QAbstractItemModel::rowsRemoved,
                                           this, &PropertiesBrowser::rowsRemoved);

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

void PropertiesBrowser::worldRowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    if(!_worldModel->objectIndex(_propertiesBrowserModel->object()).isValid())
        _propertiesBrowserModel->setObject(NULL);
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
    qDebug() << "doubleClicked" << endl;
    if(_propertiesBrowserModel->rowCount(index) > 0) {
        qDebug() << "   doubleClicked!!!" << endl;
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


