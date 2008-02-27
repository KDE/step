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
class KUrl;

class MessageFrame: public QFrame
{
    Q_OBJECT

public:
    enum MessageType { Information, Warning, Error };
    enum MessageFlag { CloseButton = 1, CloseTimer = 2 };

    MessageFrame(QWidget* parent = 0, Qt::WindowFlags f = 0);

    int showMessage(MessageType type, const QString& text, int flags = 0);
    int changeMessage(int id, MessageType type, const QString& text, int flags = 0);
    void closeMessage(int id);

signals:
    void linkActivated(const KUrl& url);

protected slots:
    void messageLinkActivated(const QString& link);
    void messageCloseClicked(QWidget* widget);

protected:
    QVBoxLayout*   _layout;
    QSignalMapper* _signalMapper;
    int            _lastId;
};

#endif

