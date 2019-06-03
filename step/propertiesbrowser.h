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

#ifndef STEP_PROPERTIESBROWSER_H
#define STEP_PROPERTIESBROWSER_H

#include <QDockWidget>
#include <QItemDelegate>
class PropertiesBrowserModel;
class WorldModel;
class QModelIndex;
class QAbstractItemModel;
class KComboBox;
class KColorButton;
class QLineEdit;

class PropertiesBrowserDelegate: public QItemDelegate
{
    Q_OBJECT
public:
    explicit PropertiesBrowserDelegate(QObject* parent = 0):
            QItemDelegate(parent), _editor(NULL), _updating(false) {}
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                           const QModelIndex& index) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget* editor, const QModelIndex& index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                       const QModelIndex& index) const Q_DECL_OVERRIDE;
protected slots:
    void editorActivated();

protected:
    enum { SolverChoiser, ColorChoiser,
           BoolChoiser, Standard } _editorType; 
    QWidget* _editor;
    KComboBox*    _comboBox;
    KColorButton* _colorButton;
    QLineEdit*    _lineEdit;
    mutable bool _updating;
};

class PropertiesBrowserView;
class PropertiesBrowser: public QDockWidget
{
    Q_OBJECT

public:
    explicit PropertiesBrowser(WorldModel* worldModel, QWidget* parent = 0);

public slots:
    void settingsChanged();

protected slots:
    void worldModelReset();
    void worldDataChanged(bool dynamicOnly);
    void worldCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void worldRowsRemoved(const QModelIndex& parent, int start, int end);

    void currentChanged(const QModelIndex& current, const QModelIndex& previous);
    void rowsInserted(const QModelIndex& parent, int start, int end);
    void rowsRemoved(const QModelIndex& parent, int start, int end);

protected:
    bool eventFilter(QObject* object, QEvent* event) Q_DECL_OVERRIDE;

    WorldModel* _worldModel;
    PropertiesBrowserModel* _propertiesBrowserModel;
    PropertiesBrowserView* _treeView;
};

#endif

