/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "messageframe.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QSignalMapper>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

MessageFrame::MessageFrame(QWidget* parent)
    : QFrame(parent), _lastId(0)
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
}

int MessageFrame::showMessage(Type type, const QString& text, Flags flags)
{
    if(_layout->count() != 0) {
        QFrame* line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        _layout->addWidget(line);
    }

    QString widgetName(QStringLiteral("message"));
    widgetName.append(QString::number(_lastId));

    QWidget* widget = new QWidget(this);
    widget->setObjectName(widgetName);
    widget->setMinimumHeight(32);

    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0,2,0,2);

    QLabel* iconLabel = new QLabel(widget);
    iconLabel->setObjectName(QStringLiteral("iconLabel"));
    if(type == Error) iconLabel->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-error")).pixmap(16,16));
    else if(type == Warning) iconLabel->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-warning")).pixmap(16,16));
    else iconLabel->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-information")).pixmap(16,16));
    layout->addWidget(iconLabel);

    QLabel* textLabel = new QLabel(widget);
    textLabel->setObjectName(QStringLiteral("textLabel"));
    //textLabel->setWordWrap(true);
    textLabel->setText(text);
    layout->addWidget(textLabel, 1);

    connect(textLabel, &QLabel::linkActivated,
                this, &MessageFrame::messageLinkActivated);

    if(flags.testFlag(CloseButton)) {
        QToolButton* button = new QToolButton(widget);
        button->setObjectName(QStringLiteral("closeButton"));
        button->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));
        button->setIconSize(QSize(16,16));
        button->setAutoRaise(true);
        layout->addWidget(button);

        connect(button, &QToolButton::clicked, this, [this, widget](){ messageCloseClicked(widget); });
    }

    if(flags.testFlag(CloseTimer)) {
        QTimer* timer = new QTimer(widget);
        timer->setObjectName(QStringLiteral("closeTimer"));
        timer->setSingleShot(true);
        timer->setInterval(2000);

        connect(timer, &QTimer::timeout, this, [this, widget](){ messageCloseClicked(widget); });
        timer->start();
    }

    _layout->addWidget(widget);
    if(!isVisible()) show();

    return _lastId++;
}

int MessageFrame::changeMessage(int id, Type type, const QString& text, Flags flags)
{
    QString widgetName(QStringLiteral("message"));
    widgetName.append(QString::number(id));
    QWidget* widget = findChild<QWidget*>(widgetName);
    if(widget) messageCloseClicked(widget);
    return showMessage(type, text, flags);
}

void MessageFrame::closeMessage(int id)
{
    QString widgetName(QStringLiteral("message"));
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

    _layout->itemAt(index)->widget()->hide();
    _layout->takeAt(index)->widget()->deleteLater();
    if(index > 0) {
        _layout->itemAt(index-1)->widget()->hide();
        _layout->takeAt(index-1)->widget()->deleteLater();
    } else if(_layout->count() > 0) {
        _layout->itemAt(0)->widget()->hide();
        _layout->takeAt(0)->widget()->deleteLater();
    }

    if(_layout->count() == 0) hide();
}

#include "moc_messageframe.cpp"
