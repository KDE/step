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

    explicit MessageFrame(QWidget* parent = 0);

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

