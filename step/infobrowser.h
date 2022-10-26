/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    void showEvent(QShowEvent* event) override;
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
