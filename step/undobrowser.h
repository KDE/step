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

#ifndef STEP_UNDOBROWSER_H
#define STEP_UNDOBROWSER_H

#include <QDockWidget>

class WorldModel;
class QUndoView;
class QUrl;

class UndoBrowser: public QDockWidget
{
    Q_OBJECT

public:
    explicit UndoBrowser(WorldModel* worldModel, QWidget* parent = 0);

public slots:
    void setEmptyLabel(const QString& label);
    void setCurrentFileUrl(const QUrl& url);
    void setUndoEnabled(bool enabled);

protected:
    WorldModel* _worldModel;
    QUndoView*  _undoView;
};

#endif

