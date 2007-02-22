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

#ifndef STEP_MAINWINDOW_H
#define STEP_MAINWINDOW_H

#include <KMainWindow>

class WorldModel;
class WorldBrowser;
class WorldScene;
class WorldGraphicsView;
class PropertiesBrowser;
class ItemPalette;
class QTimer;
class KAction;

class MainWindow : public KMainWindow
{
    Q_OBJECT

public:
    MainWindow();

public slots:
    bool newFile();
    bool openFile(const QString& name = QString());
    bool saveFileAs(const QString& name = QString());
    bool saveFile();

    void setModified(bool modified);

    void simulationStartStop();
    void simulationStart();
    void simulationStop();

protected slots:
    void simulationFrame();

    /*
protected slots:
    void on_actionNew_triggered(bool checked);
    void on_actionOpen_triggered(bool checked);
    void on_actionSave_triggered(bool checked);
    void on_actionSaveAs_triggered(bool checked);

    void on_actionStep_triggered(bool checked);
    void on_actionSimulation_triggered(bool checked);

    void on_actionAboutStep_triggered(bool checked);

    void on_simulationTimer_timeout();
    */

protected:
    void setupActions();
    void updateCaption();
    bool queryClose();
    bool maybeSave();

protected:
    WorldModel*         worldModel;
    WorldBrowser*       worldBrowser;
    PropertiesBrowser*  propertiesBrowser;
    ItemPalette*        itemPalette;
    WorldScene*         worldScene;
    WorldGraphicsView*  worldGraphicsView;
    QTimer*             simulationTimer;

    KAction* actionSimulation;
    KAction* actionSimulationStart;
    KAction* actionSimulationStop;

    KAction* actionUndo;
    KAction* actionRedo;

    QString currentFileName;
    bool    modified;

    static const int FPS = 25; //XXX
};

#endif

