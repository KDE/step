/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit UndoBrowser(WorldModel* worldModel, QWidget* parent = nullptr);

public slots:
    void setEmptyLabel(const QString& label);
    void setCurrentFileUrl(const QUrl& url);
    void setUndoEnabled(bool enabled);

protected:
    WorldModel* _worldModel;
    QUndoView*  _undoView;
};

#endif

