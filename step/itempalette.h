/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STEP_ITEMPALETTE_H
#define STEP_ITEMPALETTE_H

#include <QDockWidget>
#include <QList>

class WorldModel;
class QToolButton;
class QScrollArea;
class QAction;
class QActionGroup;
class PaletteLayout;

namespace StepCore { class MetaObject; }

class ItemPalette: public QDockWidget
{
    Q_OBJECT

public:
    explicit ItemPalette(WorldModel* worldModel, QWidget* parent = nullptr);

signals:
    void beginAddItem(const QString& name);

public slots:
    void endAddItem(const QString& name, bool success);

protected slots:
    void actionTriggered(QAction* action);
    void showButtonTextToggled(bool b);

protected:
    void createSeparator();
    void createToolButton(QAction* action);
    void createObjectAction(const StepCore::MetaObject* metaObject);

    bool event(QEvent* event) override;

    WorldModel*     _worldModel;
    QScrollArea*    _scrollArea;
    QWidget*        _widget;
    PaletteLayout*  _layout;

    QAction*        _pointerAction;
    QActionGroup*   _actionGroup;

    QList<QToolButton*> _toolButtons;
};

#endif

