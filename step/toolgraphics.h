/* This file is part of Step.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
   Copyright (C) 2014 Inge Wallin        <inge@lysator.liu.se>

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
#include "stepgraphicsitem.h"

#include <QGraphicsTextItem>
#include <QPointer>
#include <QWidget>

#include <KComboBox>
#include <KTextEdit>

#include <limits.h>

#include <stepcore/tool.h>

class KPlotWidget;
class KPlotObject;
class KToggleAction;
class QAction;
class QDialog;
class QDialogButtonBox;
class QAbstractButton;
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

/////////////////////////////////////////////////////////////////////////////////////////

class WidgetVertexHandlerGraphicsItem: public OnHoverHandlerGraphicsItem
{
    Q_OBJECT

public:
    WidgetVertexHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                        QGraphicsItem* parent, int vertexNum)
        : OnHoverHandlerGraphicsItem(item, worldModel, parent, NULL, NULL, vertexNum) {}


protected:
    void setValue(const StepCore::Vector2d& value) Q_DECL_OVERRIDE;
    StepCore::Vector2d value() Q_DECL_OVERRIDE;
};

class WidgetGraphicsItem: public QObject, public StepGraphicsItem
{
    Q_OBJECT

public:
    WidgetGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    ~WidgetGraphicsItem();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

    void stateChanged() Q_DECL_OVERRIDE;
    void viewScaleChanged() Q_DECL_OVERRIDE;
    void worldDataChanged(bool dynamicOnly) Q_DECL_OVERRIDE;

protected:
    void setCenteralWidget(QWidget* widget);
    QWidget* centeralWidget() const { return _centralWidget; }

    const QBrush& backgroundBrush() const { return _backgroundBrush; }
    void setBackgroundBrush(const QBrush& brush) { _backgroundBrush = brush; }

    void mouseSetPos(const QPointF& pos, const QPointF&, MovingState movingState) Q_DECL_OVERRIDE;
    OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF& pos) Q_DECL_OVERRIDE;

    QPointer<QWidget> _centralWidget;
    QBrush   _backgroundBrush;
};

/////////////////////////////////////////////////////////////////////////////////////////

class NoteGraphicsItem;
class NoteTextEdit: public KTextEdit
{
    Q_OBJECT

public:
    explicit NoteTextEdit(NoteGraphicsItem* noteItem, QWidget* parent = 0)
        : KTextEdit(parent), _noteItem(noteItem), _mousePressPoint(-1,-1) {}
    QString emptyNotice() const;

protected:
    StepCore::NoteFormula* formulaAt(const QPoint& pos);
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

    NoteGraphicsItem* _noteItem;
    QPoint _mousePressPoint;
};

class KToolBar;
class KSelectAction;
class KFontAction;
class KFontSizeAction;
class NoteGraphicsItem: public WidgetGraphicsItem
{
    Q_OBJECT

public:
    NoteGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    void worldDataChanged(bool dynamicOnly) Q_DECL_OVERRIDE;

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
    bool eventFilter(QObject* obj, QEvent* event) Q_DECL_OVERRIDE;

    StepCore::Note* note() const;
    bool            _hasFocus;

    QList<StepCore::Item*> _newItems;

    QWidget*        _widget;
    NoteTextEdit*   _textEdit;
    KToolBar*       _toolBar;

    QAction *       _actionColor;
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

    QAction * _actionInsertImage;
    QAction * _actionInsertFormula;

    friend class NoteTextEdit;
};

/////////////////////////////////////////////////////////////////////////////////////////

class DataSourceWidget: public QWidget
{
    Q_OBJECT

public:
    explicit DataSourceWidget(QWidget* parent = 0);

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

/////////////////////////////////////////////////////////////////////////////////////////

class GraphGraphicsItem: public WidgetGraphicsItem
{
public:
    GraphGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    void worldDataChanged(bool) Q_DECL_OVERRIDE;

protected:
    StepCore::Graph* graph() const;
    void adjustLimits();

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

    void populateMenu(QMenu* menu, KActionCollection* actions) Q_DECL_OVERRIDE;

protected slots:
    void clearGraph();
    void configureGraph();
    void confApply(QAbstractButton *button);
    void confChanged();

protected:
    StepCore::Graph* graph() const;
    Ui::WidgetConfigureGraph* _confUi;
    QDialog*                  _confDialog;
    QDialogButtonBox         *_buttonBox;
    bool                      _confChanged;
};

/////////////////////////////////////////////////////////////////////////////////////////

class QLCDNumber;
class QFrame;
class MeterGraphicsItem: public WidgetGraphicsItem
{
public:
    MeterGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    void worldDataChanged(bool) Q_DECL_OVERRIDE;

protected:
    StepCore::Meter* meter() const;

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

    void populateMenu(QMenu* menu, KActionCollection* actions) Q_DECL_OVERRIDE;

protected slots:
    void configureMeter();
    void confApply(QAbstractButton *button);
    void confChanged();

protected:
    StepCore::Meter* meter() const;
    QAction * _configureAction;
    Ui::WidgetConfigureMeter *_confUi;
    QDialog                  *_confDialog;
    QDialogButtonBox         *_buttonBox;
    bool                      _confChanged;
};

/////////////////////////////////////////////////////////////////////////////////////////

class ControllerGraphicsItem: public WidgetGraphicsItem
{
    Q_OBJECT

public:
    ControllerGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    void worldDataChanged(bool) Q_DECL_OVERRIDE;

protected slots:
    void incTriggered();
    void decTriggered();
    void sliderChanged(int value);
    void sliderReleased();

protected:
    StepCore::Controller* controller() const;

    double _lastValue;
    bool _changed;

    QWidget* _widget;
    QSlider* _slider;
    QLabel*  _labelMin;
    QLabel*  _labelMax;
    QLabel*  _labelSource;

    QAction * _incAction;
    QAction * _decAction;
    QString  _incShortcut;
    QString  _decShortcut;

    static const int SLIDER_MIN = 0;
    static const int SLIDER_MAX = INT_MAX-100;
};

class ControllerMenuHandler: public ItemMenuHandler
{
    Q_OBJECT

public:
    ControllerMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
        : ItemMenuHandler(object, worldModel, parent) {}

    void populateMenu(QMenu* menu, KActionCollection* actions) Q_DECL_OVERRIDE;

protected slots:
    void incTriggered();
    void decTriggered();
    void configureController();
    void confApply(QAbstractButton *button);
    void confChanged();

protected:
    StepCore::Controller* controller() const;
    QAction * _configureAction;
    Ui::WidgetConfigureController *_confUi;
    QDialog                       *_confDialog;
    QDialogButtonBox              *_buttonBox;
    bool                           _confChanged;
};

/////////////////////////////////////////////////////////////////////////////////////////

class TracerCreator: public AttachableItemCreator
{
public:
    TracerCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
        : AttachableItemCreator(className, worldModel, worldScene,
                            WorldScene::SnapRigidBody | WorldScene::SnapParticle |
                            WorldScene::SnapSetLocalPosition, 0) {}
};

class TracerGraphicsItem: public StepGraphicsItem
{
public:
    TracerGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;

    void viewScaleChanged() Q_DECL_OVERRIDE;
    void worldDataChanged(bool dynamicOnly) Q_DECL_OVERRIDE;

protected:
    void mouseSetPos(const QPointF& pos, const QPointF&, MovingState movingState) Q_DECL_OVERRIDE;
    StepCore::Tracer* tracer() const;
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

    void populateMenu(QMenu* menu, KActionCollection* actions) Q_DECL_OVERRIDE;

protected slots:
    void clearTracer();
};


#endif

