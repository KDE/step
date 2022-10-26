/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STEP_MESSAGEFRAME_H
#define STEP_MESSAGEFRAME_H

#include <QFrame>

class QVBoxLayout;
class QSignalMapper;


class MessageFrame: public QFrame
{
    Q_OBJECT

public:
    enum Type { Information, Warning, Error };
    enum Flag { CloseButton = 1, CloseTimer = 2 };
    Q_DECLARE_FLAGS(Flags, Flag)

    explicit MessageFrame(QWidget* parent = nullptr);

    int showMessage(Type type, const QString& text, Flags flags = {});
    int changeMessage(int id, Type type, const QString& text, Flags flags = {});
    void closeMessage(int id);

signals:
    void linkActivated(const QString& url);

protected slots:
    void messageLinkActivated(const QString& link);
    void messageCloseClicked(QWidget* widget);

protected:
    QVBoxLayout*   _layout;
    QSignalMapper* _signalMapper;
    int            _lastId;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MessageFrame::Flags)

#endif

