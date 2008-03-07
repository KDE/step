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

#include "messageframe.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSignalMapper>
#include <QToolButton>
#include <QLabel>
#include <QTimer>
#include <KUrl>
#include <KIcon>
#include <KDebug>

MessageFrame::MessageFrame(QWidget* parent, Qt::WindowFlags f)
    : QFrame(parent, f), _lastId(0)
{
    hide();

    int br, bg, bb;
    setFrameShape(QFrame::StyledPanel);
    palette().color(QPalette::Window).getRgb(&br, &bg, &bb);
    setStyleSheet(QString(".MessageFrame {border: 2px solid rgba(133,133,133,85%);"
        "border-radius: 6px; background-color: rgba(%1,%2,%3,85%);}").arg(br).arg(bg).arg(bb));

    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(9,0,9,0);
    _layout->setSpacing(0);
    _layout->setSizeConstraint(QLayout::SetFixedSize);

    _signalMapper = new QSignalMapper(this);
    connect(_signalMapper, SIGNAL(mapped(QWidget*)),
                this, SLOT(messageCloseClicked(QWidget*)));
}

int MessageFrame::showMessage(Type type, const QString& text, Flags flags)
{
    if(_layout->count() != 0) {
        QFrame* line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        _layout->addWidget(line);
    }

    QString widgetName("message");
    widgetName.append(QString::number(_lastId));

    QWidget* widget = new QWidget(this);
    widget->setObjectName(widgetName);
    widget->setMinimumHeight(32);

    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0,2,0,2);

    QLabel* iconLabel = new QLabel(widget);
    iconLabel->setObjectName("iconLabel");
    if(type == Error) iconLabel->setPixmap(KIcon("dialog-error").pixmap(16,16));
    else if(type == Warning) iconLabel->setPixmap(KIcon("dialog-warning").pixmap(16,16));
    else iconLabel->setPixmap(KIcon("dialog-information").pixmap(16,16));
    layout->addWidget(iconLabel);

    QLabel* textLabel = new QLabel(widget);
    textLabel->setObjectName("textLabel");
    textLabel->setText(text);
    layout->addWidget(textLabel, 1);

    connect(textLabel, SIGNAL(linkActivated(const QString&)),
                this, SLOT(messageLinkActivated(const QString&)));

    if(flags.testFlag(CloseButton)) {
        QToolButton* button = new QToolButton(widget);
        button->setObjectName("closeButton");
        button->setIcon(KIcon("window-close"));
        button->setIconSize(QSize(16,16));
        button->setAutoRaise(true);
        layout->addWidget(button);

        _signalMapper->setMapping(button, widget);
        connect(button, SIGNAL(clicked()), _signalMapper, SLOT(map()));
    }

    if(flags.testFlag(CloseTimer)) {
        QTimer* timer = new QTimer(widget);
        timer->setObjectName("closeTimer");
        timer->setSingleShot(true);
        timer->setInterval(2000);

        _signalMapper->setMapping(timer, widget);
        connect(timer, SIGNAL(timeout()), _signalMapper, SLOT(map()));
        timer->start();
    }

    _layout->addWidget(widget);
    if(!isVisible()) show();

    return _lastId++;
}

int MessageFrame::changeMessage(int id, Type type, const QString& text, Flags flags)
{
    QString widgetName("message");
    widgetName.append(QString::number(id));
    QWidget* widget = findChild<QWidget*>(widgetName);
    if(widget) messageCloseClicked(widget);
    return showMessage(type, text, flags);
}

void MessageFrame::closeMessage(int id)
{
    QString widgetName("message");
    widgetName.append(QString::number(id));
    QWidget* widget = findChild<QWidget*>(widgetName);
    if(widget) messageCloseClicked(widget);
}
    
void MessageFrame::messageLinkActivated(const QString& link)
{
    emit linkActivated(link);
}

void MessageFrame::messageCloseClicked(QWidget* widget)
{
    int index = _layout->indexOf(widget);
    if(index < 0) return;

    _layout->takeAt(index)->widget()->deleteLater();
    if(index > 0) {
        _layout->takeAt(index-1)->widget()->deleteLater();
    } else if(_layout->count() > 0) {
        _layout->takeAt(0)->widget()->deleteLater();
    }

    if(_layout->count() == 0) hide();
}

