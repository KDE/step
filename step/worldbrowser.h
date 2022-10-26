/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STEP_WORLDBROWSER_H
#define STEP_WORLDBROWSER_H

#include <QDockWidget>

class WorldModel;
class WorldBrowserView;

class WorldBrowser: public QDockWidget
{
    Q_OBJECT

public:
    explicit WorldBrowser(WorldModel* worldModel, QWidget* parent = nullptr);

protected:
    WorldBrowserView* _worldBrowserView;
};

#endif

