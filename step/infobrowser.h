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
#include <QList>
#include <KUrl>

class WorldModel;
class QModelIndex;
class QShowEvent;
class QAction;
class KToolBar;
class KHTMLPart;
class KJob;
class KUrl;

class InfoBrowser: public QDockWidget
{
    Q_OBJECT

public:
    explicit InfoBrowser(WorldModel* worldModel, QWidget* parent = 0, Qt::WindowFlags flags = 0);

protected slots:
    void worldCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void openUrl(const KUrl& url, bool clearHistory = false, bool fromHistory = false);
    void setHtml(const QString& data, bool fromHistory = false, const KUrl& url = KUrl());
    void wikiResult(KJob* job);

    void back();
    void forward();
    void openInBrowser();
    void settingsChanged();

protected:
    void showEvent(QShowEvent* event);

    WorldModel* _worldModel;

    KToolBar*   _toolBar;
    KHTMLPart*  _htmlPart;

    KJob*       _wikiJob;
    KUrl        _wikiUrl;
    bool        _wikiFromHistory;

    QAction*    _execAction;
    QAction*    _backAction;
    QAction*    _forwardAction;
    QStringList _backHistory;
    QStringList _forwardHistory;

    bool        _selectionChanged;
};


#endif
