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

#include "itempalette.h"

#include "worldmodel.h"
#include "worldfactory.h"
#include <stepcore/world.h>

#include "settings.h"

#include <QAction>
#include <QActionGroup>
#include <QEvent>
#include <QIcon>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyleOption>
#include <QToolButton>
#include <QVBoxLayout>

#include <KLocalizedString>

class QPaintEvent;

// Inspired by QToolBarSeparator
class Separator: public QWidget
{
public:
    explicit Separator(QWidget* parent): QWidget(parent) {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        setProperty("isSeparator", true);
    }

    QSize sizeHint() const Q_DECL_OVERRIDE {
        QStyleOption opt; opt.initFrom(this);
        const int extent = style()->pixelMetric(QStyle::PM_ToolBarSeparatorExtent, &opt, parentWidget());
        return QSize(extent, extent);
    }

    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE {
        QPainter p(this); QStyleOption opt; opt.initFrom(this);
        style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &opt, &p, parentWidget());
    }
};

class PaletteLayout: public QLayout
{
public:
    PaletteLayout(QWidget *parent, int margin = 0, int spacing = -1)
        : QLayout(parent) { setContentsMargins(margin, margin, margin, margin); setSpacing(spacing); resetCache(); }
    PaletteLayout(int spacing = -1) { setSpacing(spacing); resetCache(); }
    ~PaletteLayout() { QLayoutItem *item; while ((item = takeAt(0))) delete item; }

    void addItem(QLayoutItem *item) Q_DECL_OVERRIDE { itemList.append(item); resetCache(); }
    int count() const Q_DECL_OVERRIDE { return itemList.size(); }
    QLayoutItem* itemAt(int index) const Q_DECL_OVERRIDE { return itemList.value(index); }
    QLayoutItem* takeAt(int index) Q_DECL_OVERRIDE {
        resetCache();
        if (index >= 0 && index < itemList.size()) return itemList.takeAt(index);
        else return 0;
    }

    Qt::Orientations expandingDirections() const Q_DECL_OVERRIDE { return Qt::Vertical; }
    bool hasHeightForWidth() const Q_DECL_OVERRIDE { return true; }

    int heightForWidth(int width) const Q_DECL_OVERRIDE {
        if(isCachedHeightForWidth && cachedHeightForWidth.width() == width) {
            return cachedHeightForWidth.height();
        } else {
            cachedHeightForWidth.setWidth(width);
            cachedHeightForWidth.setHeight(doLayout(QRect(0, 0, width, 0), true));
            isCachedHeightForWidth = true;
            return cachedHeightForWidth.height();
        }
    }
    
    void setGeometry(const QRect &rect) Q_DECL_OVERRIDE {
        resetCache(); QLayout::setGeometry(rect); doLayout(rect, false);
    }

    QSize sizeHint() const Q_DECL_OVERRIDE { return minimumSize(); }

    QSize minimumSize() const Q_DECL_OVERRIDE {
        if(isCachedMinimumSize) return cachedMinimumSize;
        cachedMinimumSize = QSize();
        QLayoutItem *item;
        foreach (item, itemList)
            cachedMinimumSize = cachedMinimumSize.expandedTo(item->minimumSize());
        isCachedMinimumSize = true;
        return cachedMinimumSize;
    }

    void setOneLine(bool b) { oneLine = b; invalidate(); }
    bool isOneLine() const { return oneLine; }

    void invalidate() Q_DECL_OVERRIDE { resetCache(); QLayout::invalidate(); }

protected:
    void resetCache() { isCachedMinimumSize = false; isCachedHeightForWidth = false; }

    int doLayout(const QRect &rect, bool testOnly) const
    {
        int x = rect.x();
        int y = rect.y();
        int lineHeight = 0;

        if(oneLine) {
            foreach(QLayoutItem* item, itemList) {
                y = y + lineHeight + spacing();
                lineHeight = item->sizeHint().height();
                if(!testOnly)
                    item->setGeometry(QRect(rect.x(), y, rect.width(), lineHeight));
            }
        } else {
            foreach(QLayoutItem* item, itemList) {
                int w = item->sizeHint().width(); int h = item->sizeHint().height();
                int nextX = x + item->sizeHint().width() + spacing();
                if(item->widget() && item->widget()->property("isSeparator").toBool()) {
                    x = rect.x();
                    y = y + lineHeight + spacing();
                    nextX = x + rect.width();
                    w = rect.width();
                    lineHeight = 0;
                } else if(nextX - spacing() > rect.right() && lineHeight > 0) {
                    x = rect.x();
                    y = y + lineHeight + spacing();
                    nextX = x + w + spacing();
                    lineHeight = 0;
                }

                if(!testOnly) item->setGeometry(QRect(x, y, w, h));

                x = nextX;
                lineHeight = qMax(lineHeight, h);
            }
        }
        return y + lineHeight - rect.y();
    }

    QList<QLayoutItem *> itemList;
    bool oneLine;

    mutable bool isCachedMinimumSize;
    mutable bool isCachedHeightForWidth;
    mutable QSize cachedMinimumSize;
    mutable QSize cachedHeightForWidth;
};

class PaletteScrollArea: public QScrollArea
{
public:
    PaletteScrollArea(QWidget* parent): QScrollArea(parent) {}

protected:
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE {
        if(widget() && widget()->layout()) {
            QSize size(maximumViewportSize().width(),
                    widget()->layout()->heightForWidth(maximumViewportSize().width()));
            if(size.height() > maximumViewportSize().height()) {
                int ext = style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
                size.setWidth(maximumViewportSize().width() - 
                                verticalScrollBar()->sizeHint().width() - ext);
                size.setHeight(widget()->layout()->heightForWidth(size.width()));
            }
            widget()->resize(size);
        }
        QScrollArea::resizeEvent(event);
    }
};

ItemPalette::ItemPalette(WorldModel* worldModel, QWidget* parent)
    : QDockWidget(i18n("Palette"), parent), _worldModel(worldModel), _widget(0)
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    QWidget* topWidget = new QWidget(this);

    _scrollArea = new PaletteScrollArea(topWidget);
    _scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _scrollArea->setFrameShape(QFrame::NoFrame);

    _widget = new QWidget(_scrollArea);
    _layout = new PaletteLayout(_widget);
    _layout->setSpacing(0);
    _layout->setOneLine(Settings::showButtonText());

    _actionGroup = new QActionGroup(this);
    _actionGroup->setExclusive(true);

    _pointerAction = new QAction(i18n("Pointer"), this);
    _pointerAction->setToolTip(i18n("Selection pointer"));
    _pointerAction->setIcon(QIcon::fromTheme(QStringLiteral("pointer")));
    _pointerAction->setCheckable(true);
    _pointerAction->setChecked(true);
    _pointerAction->setProperty("step_object", "Pointer");
    _actionGroup->addAction(_pointerAction);
    createToolButton(_pointerAction);
    createSeparator();

    foreach(const QString &name, _worldModel->worldFactory()->paletteMetaObjects()) {
        if(!name.isEmpty()) createObjectAction(_worldModel->worldFactory()->metaObject(name));
        else createSeparator();
    }

    _scrollArea->setWidget(_widget);
    _scrollArea->setMinimumWidth(_widget->minimumSizeHint().width());

    QVBoxLayout* topLayout = new QVBoxLayout(topWidget);
    topLayout->addWidget(_scrollArea);
    setWidget(topWidget);

    QObject::connect(_actionGroup, &QActionGroup::triggered, this, &ItemPalette::actionTriggered);

    QAction* showText = new QAction(i18n("Show text"), this);
    showText->setCheckable(true);
    showText->setChecked(Settings::showButtonText());
    QObject::connect(showText, &QAction::toggled, this, &ItemPalette::showButtonTextToggled);

    _widget->addAction(showText);
    _widget->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void ItemPalette::createToolButton(QAction* action)
{
    QToolButton* button = new QToolButton(_widget);
    button->setToolButtonStyle(Settings::showButtonText() ? 
                    Qt::ToolButtonTextBesideIcon : Qt::ToolButtonIconOnly);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    button->setAutoRaise(true);
    button->setIconSize(QSize(22,22));
    button->setDefaultAction(action);
    _toolButtons.append(button);
    _layout->addWidget(button);
}

void ItemPalette::createSeparator()
{
    QAction* action = new QAction(this);
    action->setSeparator(true);
    _actionGroup->addAction(action);
    _layout->addWidget(new Separator(_widget));
}

void ItemPalette::createObjectAction(const StepCore::MetaObject* metaObject)
{
    Q_ASSERT(metaObject && !metaObject->isAbstract());

    QAction* action = new QAction(metaObject->classNameTr(), this);
    action->setToolTip(metaObject->descriptionTr());
    action->setIcon(_worldModel->worldFactory()->objectIcon(metaObject));
    action->setCheckable(true);
    action->setProperty("step_object", metaObject->className());
    _actionGroup->addAction(action);
    createToolButton(action);
}

void ItemPalette::showButtonTextToggled(bool b)
{
    Settings::setShowButtonText(b);
    Settings::self()->save();
    foreach(QToolButton* button, _toolButtons) {
        button->setToolButtonStyle(b ? Qt::ToolButtonTextBesideIcon :
                                       Qt::ToolButtonIconOnly);
    }
    _layout->setOneLine(b);
    _scrollArea->setMinimumWidth(_widget->minimumSizeHint().width());
}

void ItemPalette::actionTriggered(QAction* action)
{
    emit beginAddItem(action->property("step_object").toString());
}

void ItemPalette::endAddItem(const QString& name, bool /*success*/)
{
    if(name == _actionGroup->checkedAction()->property("step_object").toString())
        _pointerAction->setChecked(true);
}

bool ItemPalette::event(QEvent* event)
{
    return QDockWidget::event(event);
}

