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

#ifndef STEP_TOOLGRAPHICS_H
#define STEP_TOOLGRAPHICS_H

#include "worldgraphics.h"
#include <QGraphicsTextItem>
#include <QAbstractItemModel>
#include <QWidget>
#include <KTextEdit>
#include <KComboBox>
#include <limits.h>

#include <stepcore/tool.h>

class KPlotWidget;
class KPlotObject;
class KToggleAction;
class KAction;
class KDialog;
class QSlider;
class QLabel;

namespace Ui {
    class WidgetConfigureGraph;
    class WidgetConfigureMeter;
    class WidgetConfigureController;
}

namespace StepCore {
    class Note;
    class Meter;
    class Graph;
    class Controller;
    class Tracer;
}

class NoteGraphicsItem;
class NoteTextEdit: public KTextEdit
{
    Q_OBJECT

public:
    NoteTextEdit(NoteGraphicsItem* noteItem, QWidget* parent = 0)
        : KTextEdit(parent), _noteItem(noteItem), _mousePressPoint(-1,-1) {}
    QString emptyNotice() const;

protected:
    StepCore::NoteFormula* formulaAt(const QPoint& pos);
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

    NoteGraphicsItem* _noteItem;
    QPoint _mousePressPoint;
};

class KToolBar;
class KSelectAction;
class KFontAction;
class KFontSizeAction;
class NoteGraphicsItem: public QObject, public WorldGraphicsItem
{
    Q_OBJECT

public:
    NoteGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    ~NoteGraphicsItem();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void stateChanged();
    void viewScaleChanged();
    void worldDataChanged(bool dynamicOnly);

protected slots:
    void formatColor();
    void formatBold(bool checked);
    void formatAlign(QAction* action);
    void formatFontFamily(const QString& family);
    void formatFontSize(int size);
    void currentCharFormatChanged(const QTextCharFormat& f);
    void cursorPositionChanged();
    void insertImage();
    void insertFormula();

protected:
    bool checkLatex();
    bool editFormula(StepCore::NoteFormula* formula);
    bool eventFilter(QObject* obj, QEvent* event);

    StepCore::Note* note() const;
    double          _lastScale;
    bool            _hasFocus;

    QList<StepCore::Item*> _newItems;

    QWidget*        _widget;
    NoteTextEdit*   _textEdit;
    KToolBar*       _toolBar;

    KAction*       _actionColor;
    KToggleAction* _actionBold;
    KToggleAction* _actionItalic;
    KToggleAction* _actionUnderline;

    KSelectAction* _actionAlign;
    KToggleAction* _actionAlignLeft;
    KToggleAction* _actionAlignCenter;
    KToggleAction* _actionAlignRight;
    KToggleAction* _actionAlignJustify;

    KFontAction*        _actionFont;
    KFontSizeAction*    _actionFontSize;

    KAction* _actionInsertImage;
    KAction* _actionInsertFormula;

    friend class NoteTextEdit;
};

class DataSourceWidget: public QWidget
{
    Q_OBJECT

public:
    DataSourceWidget(QWidget* parent = 0);

    void setSkipReadOnly(bool skipReadOnly) { _skipReadOnly = skipReadOnly; }
    void setDataSource(WorldModel* worldModel, StepCore::Object* object = NULL,
                            const QString& property = QString(), int index = 0);

    StepCore::Object* dataObject() const;
    QString dataProperty() const { return _property->itemData(_property->currentIndex()).toString(); }
    int dataIndex() const { return _index->currentIndex(); }

signals:
    void dataSourceChanged();

protected slots:
    void objectSelected(int index);
    void propertySelected(int index);

protected:
    void addObjects(const QModelIndex& parent, const QString& indent);

    WorldModel* _worldModel;

    KComboBox*  _object;
    KComboBox*  _property;
    KComboBox*  _index;

    bool _skipReadOnly;
};

class GraphGraphicsItem: public QObject, public WorldGraphicsItem
{
    Q_OBJECT

public:
    GraphGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    ~GraphGraphicsItem();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void stateChanged();
    void viewScaleChanged();
    void worldDataChanged(bool);

protected:
    StepCore::Graph* graph() const;
    void adjustLimits();

    double _lastScale;
    double _lastPointTime;
    QRgb   _lastColor;

    KPlotWidget* _plotWidget;
    KPlotObject* _plotObject;
    KPlotObject* _plotObject1;
};

class GraphMenuHandler: public ItemMenuHandler
{
    Q_OBJECT

public:
    GraphMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
        : ItemMenuHandler(object, worldModel, parent) {}

    void populateMenu(QMenu* menu);

protected slots:
    void clearGraph();
    void configureGraph();
    void confApply();
    void confChanged();

protected:
    StepCore::Graph* graph() const;
    Ui::WidgetConfigureGraph* _confUi;
    KDialog*                  _confDialog;
    bool                      _confChanged;
};

class QLCDNumber;
class QFrame;
class MeterGraphicsItem: public QObject, public WorldGraphicsItem
{
    Q_OBJECT

public:
    MeterGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    ~MeterGraphicsItem();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void stateChanged();
    void viewScaleChanged();
    void worldDataChanged(bool);

protected:
    StepCore::Meter* meter() const;

    double _lastScale;
    double _lastValue;

    QFrame*     _widget;
    QLCDNumber* _lcdNumber;
    QLabel*     _labelUnits;

    /*
    QLabel*  _labelMin;
    QLabel*  _labelMax;
    QLabel*  _labelSource;
    */
};

class MeterMenuHandler: public ItemMenuHandler
{
    Q_OBJECT

public:
    MeterMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
        : ItemMenuHandler(object, worldModel, parent) {}

    void populateMenu(QMenu* menu);

protected slots:
    void configureMeter();
    void confApply();
    void confChanged();

protected:
    StepCore::Meter* meter() const;
    KAction* _configureAction;
    Ui::WidgetConfigureMeter* _confUi;
    KDialog* _confDialog;
    bool     _confChanged;
};

class ControllerGraphicsItem: public QObject, public WorldGraphicsItem
{
    Q_OBJECT

public:
    ControllerGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    ~ControllerGraphicsItem();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void stateChanged();
    void viewScaleChanged();
    void worldDataChanged(bool);

protected slots:
    void incTriggered();
    void decTriggered();
    void sliderChanged(int value);
    void sliderReleased();

protected:
    StepCore::Controller* controller() const;

    double _lastScale;
    double _lastValue;

    QWidget* _widget;
    QSlider* _slider;
    QLabel*  _labelMin;
    QLabel*  _labelMax;
    QLabel*  _labelSource;

    KAction* _incAction;
    KAction* _decAction;
    QString  _incShortcut;
    QString  _decShortcut;

    bool _changed;

    static const int SLIDER_MIN = 0;
    static const int SLIDER_MAX = INT_MAX-100;
};

class ControllerMenuHandler: public ItemMenuHandler
{
    Q_OBJECT

public:
    ControllerMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
        : ItemMenuHandler(object, worldModel, parent) {}

    void populateMenu(QMenu* menu);

protected slots:
    void incTriggered();
    void decTriggered();
    void configureController();
    void confApply();
    void confChanged();

protected:
    StepCore::Controller* controller() const;
    KAction* _configureAction;
    Ui::WidgetConfigureController* _confUi;
    KDialog* _confDialog;
    bool     _confChanged;
};

class TracerCreator: public ItemCreator
{
public:
    TracerCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);

protected:
    void tryAttach(const QPointF& pos);
};

class TracerGraphicsItem: public WorldGraphicsItem
{
public:
    TracerGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPainterPath shape() const;

    void viewScaleChanged();
    void worldDataChanged(bool dynamicOnly);

protected:
    StepCore::Tracer* tracer() const;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    QPolygonF _points;
    QPointF   _lastPos;
    double    _lastPointTime;
    bool      _moving;
    QPointF   _movingDelta;
};

class TracerMenuHandler: public ItemMenuHandler
{
    Q_OBJECT

public:
    TracerMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
        : ItemMenuHandler(object, worldModel, parent) {}

    void populateMenu(QMenu* menu);

protected slots:
    void clearTracer();
};


#endif

