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

#ifndef STEP_INFOBROWSER_H
#define STEP_INFOBROWSER_H

#include <QDockWidget>
#include <QTextBrowser>
#include <QUrl>

class WorldModel;
class QModelIndex;
class QShowEvent;
class QAction;

class KToolBar;
class KHTMLPart;
class KJob;

class InfoBrowser: public QDockWidget
{
    Q_OBJECT

public:
    explicit InfoBrowser(WorldModel* worldModel, QWidget* parent = nullptr);

public slots:
    void openUrl(const QUrl& url, bool clearHistory = false, bool fromHistory = false);

protected slots:
    void worldCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void setHtml(const QString& data, bool fromHistory = false, const QUrl& url = QUrl());

    void back();
    void forward();
    void openInBrowser();
    void syncSelection(bool checked = true);

protected:
    void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;
    void updateSyncSelection();

    WorldModel* _worldModel;

    KToolBar*   _toolBar;
    QTextBrowser* _htmlBrowser;

    QAction*    _followAction;
    QAction*    _syncAction;
    QAction*    _execAction;
    QAction*    _backAction;
    QAction*    _forwardAction;
    QStringList _backHistory;
    QStringList _forwardHistory;

    bool        _selectionChanged;
};


#endif
