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
#include <stepcore/xmlfile.h>

#include "settings.h"

#include <QAction>
#include <QEvent>
#include <QToolButton>
#include <QVBoxLayout>
#include <QActionGroup>
#include <QStyleOption>
#include <QScrollArea>
#include <QScrollBar>
#include <QPainter>
#include <QtAlgorithms>
#include <QFile>

#include <KMessageBox>
#include <KLocale>
#include <KIcon>

#include "itempalette.moc"

class QPaintEvent;
/*
class CustomGroup{

public:
    CustomGroup(const QString& filename, const WorldFactory* worldFactory);
    
private:
    QList<QAction> _groups;
    StepCore::World _world;
    QString _error;
};

CustomGroup::CustomGroup(const QString& filename, const WorldFactory* worldFactory){
    QFile qfile(filename);
    if(!qfile.open(QIODevice::ReadOnly | QIODevice::Text)){
        _error = i18n("Can not open file");
    };
    StepCore::XmlFile file(&qfile);
    bool ret = file.load(&_world, worldFactory);
    const StepCore::ItemList& items = _world.items();
    StepCore::ItemList::const_iterator end = items.end();
    for(StepCore::ItemList::const_iterator it = items.begin(); it != end; ++it) {
            _groups << (*it)->name();
    }
}
*/
// Inspired by QToolBarSeparator
class Separator: public QWidget
{
public:
    explicit Separator(QWidget* parent): QWidget(parent) {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        setProperty("isSeparator", true);
    }

    QSize sizeHint() const {
        QStyleOption opt; opt.initFrom(this);
        const int extent = style()->pixelMetric(QStyle::PM_ToolBarSeparatorExtent, &opt, parentWidget());
        return QSize(extent, extent);
    }

    void paintEvent(QPaintEvent *) {
        QPainter p(this); QStyleOption opt; opt.initFrom(this);
        style()->drawPrimitive(QStyle::PE_IndicatorToolBarSeparator, &opt, &p, parentWidget());
    }
};

class PaletteLayout: public QLayout
{
public:
    PaletteLayout(QWidget *parent, int margin = 0, int spacing = -1)
        : QLayout(parent) { setMargin(margin); setSpacing(spacing); resetCache(); }
    PaletteLayout(int spacing = -1) { setSpacing(spacing); resetCache(); }
    ~PaletteLayout() { QLayoutItem *item; while ((item = takeAt(0))) delete item; }

    void addItem(QLayoutItem *item) { itemList.append(item); resetCache(); }
    int count() const { return itemList.size(); }
    QLayoutItem* itemAt(int index) const { return itemList.value(index); }
    QLayoutItem* takeAt(int index) {
        resetCache();
        if (index >= 0 && index < itemList.size()) return itemList.takeAt(index);
        else return 0;
    }

    Qt::Orientations expandingDirections() const { return Qt::Vertical; }
    bool hasHeightForWidth() const { return true; }

    int heightForWidth(int width) const {
        if(isCachedHeightForWidth && cachedHeightForWidth.width() == width) {
            return cachedHeightForWidth.height();
        } else {
            cachedHeightForWidth.setWidth(width);
            cachedHeightForWidth.setHeight(doLayout(QRect(0, 0, width, 0), true));
            isCachedHeightForWidth = true;
            return cachedHeightForWidth.height();
        }
    }
    
    void setGeometry(const QRect &rect) {
        resetCache(); QLayout::setGeometry(rect); doLayout(rect, false);
    }

    QSize sizeHint() const { return minimumSize(); }

    QSize minimumSize() const {
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

    void invalidate() { resetCache(); QLayout::invalidate(); }

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
    void resizeEvent(QResizeEvent* event) {
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

ItemPalette::ItemPalette(WorldModel* worldModel, QWidget* parent, Qt::WindowFlags flags)
    : QDockWidget(i18n("Palette"), parent, flags), _worldModel(worldModel), _widget(0)
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
    _pointerAction->setIcon(KIcon("pointer"));
    _pointerAction->setCheckable(true);
    _pointerAction->setChecked(true);
    _pointerAction->setProperty("step_object", "Pointer");
    _actionGroup->addAction(_pointerAction);
    createToolButton(_pointerAction);
    createSeparator();

    _groupsActionGroup = new QActionGroup(this);
    _groupsActionGroup->setExclusive(false);

    foreach(const QString &group, _worldModel->worldFactory()->paletteGroups()) {
        QAction* action = new QAction(i18n(group.toLatin1().constData()), this);
        action->setProperty("step_group", group);
        action->setCheckable(true);
        action->setChecked(Settings::groupsToShow().contains(group));
        _groupsActionGroup->addAction(action);
        _widget->addAction(action);

        _groups.insert(group, QList<QAction*>());
        foreach(const QString &name, _worldModel->worldFactory()->paletteMetaObjects(group)) {
            if(!name.isEmpty()) _groups[group] << createObjectAction(_worldModel->worldFactory()->metaObject(name));
            else _groups[group] << createSeparator();
        }

        _groups[group] << createSeparator(); // XXX: ?
    }

    connect(_groupsActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(groupVisibilityToggled(QAction*)));

    _scrollArea->setWidget(_widget);
    _scrollArea->setMinimumWidth(_widget->minimumSizeHint().width());

    QVBoxLayout* topLayout = new QVBoxLayout(topWidget);
    topLayout->addWidget(_scrollArea);
    setWidget(topWidget);

    QObject::connect(_actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(actionTriggered(QAction*)));

    QAction* showText = new QAction(i18n("Show text"), this);
    showText->setCheckable(true);
    showText->setChecked(Settings::showButtonText());
    QObject::connect(showText, SIGNAL(toggled(bool)), this, SLOT(showButtonTextToggled(bool)));
    _widget->addAction(showText);

    QAction* customGroups = new QAction(i18n("Configure custom items"), this);
    QObject::connect(customGroups, SIGNAL(triggered(bool)), this, SLOT(configureCustomGroups()));
    _widget->addAction(customGroups);

    _widget->setContextMenuPolicy(Qt::ActionsContextMenu);

    updateGroupsVisibility();
}

ItemPalette::~ItemPalette()
{
    foreach (StepCore::World* world, _groupWorlds)
        delete world;
}

void ItemPalette::updateGroupsVisibility()
{
    foreach(const QString& group, _groups.keys()) {
        bool visible = Settings::groupsToShow().contains(group);
        foreach(QAction* action, _groups[group]) {
            //action->setVisible(visible);
            //action->property("step_widget").value<QWidget*>()->setVisible(visible);
            if(visible)
                action->property("step_widget").value<QWidget*>()->show();
            else
                action->property("step_widget").value<QWidget*>()->hide();
            kDebug() << action->text() << visible;
        }
    }
    _widget->adjustSize();
}

void ItemPalette::groupVisibilityToggled(QAction* action)
{
    QString group = action->property("step_group").toString();
    QStringList groups = Settings::groupsToShow();
    if(action->isChecked() && !groups.contains(group)) {
        groups.append(group); Settings::setGroupsToShow(groups);
    } else if(!action->isChecked()) {
        groups.removeOne(group); Settings::setGroupsToShow(groups);
    }
    Settings::self()->writeConfig();
    updateGroupsVisibility();
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
    action->setProperty("step_widget", QVariant::fromValue<QWidget*>(button));
}

QAction* ItemPalette::createSeparator()
{
    QAction* action = new QAction(this);
    action->setSeparator(true);
    _actionGroup->addAction(action);
    Separator* separator = new Separator(_widget);
    _layout->addWidget(separator);
    action->setProperty("step_widget", QVariant::fromValue<QWidget*>(separator));
    return action;
}

QAction* ItemPalette::createObjectAction(const StepCore::MetaObject* metaObject)
{
    Q_ASSERT(metaObject && !metaObject->isAbstract());

    QAction* action = new QAction(metaObject->className(), this);
    action->setToolTip(metaObject->description());
    action->setIcon(_worldModel->worldFactory()->objectIcon(metaObject));
    action->setCheckable(true);
    action->setProperty("step_object", metaObject->className());
    _actionGroup->addAction(action);
    createToolButton(action);
    return action;
}

QAction* ItemPalette::createCustomItemAction(const QString& filename, const StepCore::Item* item)
{
    kDebug() << filename << item->name();
    QAction* action = new QAction(item->name(), this);
    action->setToolTip(i18n("Create custom item: %1", item->name()));
    //action->setIcon(_worldModel->worldFactory()->objectIcon(metaObject));
    action->setCheckable(true);
    action->setProperty("step_filename", filename);
    action->setProperty("step_item", QVariant::fromValue<void*>((void*) item));
    _actionGroup->addAction(action);
    createToolButton(action);
    return action;
}

void ItemPalette::showButtonTextToggled(bool b)
{
    Settings::setShowButtonText(b);
    Settings::self()->writeConfig();
    foreach(QToolButton* button, _toolButtons) {
        button->setToolButtonStyle(b ? Qt::ToolButtonTextBesideIcon :
                                       Qt::ToolButtonIconOnly);
    }
    _layout->setOneLine(b);
    _scrollArea->setMinimumWidth(_widget->minimumSizeHint().width());
}

void ItemPalette::configureCustomGroups()
{
   loadCustomGroup("/home/kde-devel/s.step");
}

void ItemPalette::loadCustomGroup(const QString& filename)
{
    QFile qfile(filename);
    if(!qfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        KMessageBox::error(this, i18n("Can't open file"));
        return;
    }
    StepCore::XmlFile file(&qfile);
    StepCore::World* world = new StepCore::World();
    bool ret = file.load(world, _worldModel->worldFactory());
    if(!ret) {
        KMessageBox::error(this, i18n("Can't parse file"));
        return;
    }
    _groupWorlds.insert(filename, world);
    _groups.insert(filename, QList<QAction*>());

    StepCore::ItemList::const_iterator end = world->items().end();
    for(StepCore::ItemList::const_iterator it = world->items().begin(); it != end; ++it) {
        _groups[filename] << createCustomItemAction(filename, *it);
    }
    
    QAction* action = new QAction(filename, this);
    action->setProperty("step_group", filename);
    action->setCheckable(true);
    action->setChecked(true);
    _groupsActionGroup->addAction(action);
    _widget->addAction(action);

    if(!Settings::groupsToShow().contains(filename)) {
        groupVisibilityToggled(action);
    }
    updateGroupsVisibility();
}

void ItemPalette::actionTriggered(QAction* action)
{
    kDebug() << action << " " << action->property("step_object").toString() << " " << action->property("step_item").value<void*>();
    emit beginAddItem(action->property("step_object").toString(),
        static_cast<StepCore::Item*>(action->property("step_item").value<void*>()));
}

void ItemPalette::endAddItem(const QString& name, bool /*success*/, bool selectPointer)
{
    if(selectPointer) _pointerAction->setChecked(true);
}

bool ItemPalette::event(QEvent* event)
{
    return QDockWidget::event(event);
}
