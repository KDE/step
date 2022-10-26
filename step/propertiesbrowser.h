/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit PropertiesBrowserDelegate(QObject* parent = nullptr):
            QItemDelegate(parent), _editor(nullptr), _updating(false) {}
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                           const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                       const QModelIndex& index) const override;
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
    explicit PropertiesBrowser(WorldModel* worldModel, QWidget* parent = nullptr);

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
    bool eventFilter(QObject* object, QEvent* event) override;

    WorldModel* _worldModel;
    PropertiesBrowserModel* _propertiesBrowserModel;
    PropertiesBrowserView* _treeView;
};

#endif

