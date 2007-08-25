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

#include <KXmlGuiWindow>
#include <KUrl>

class WorldModel;
class WorldBrowser;
class WorldScene;
class WorldGraphicsView;
class PropertiesBrowser;
class InfoBrowser;
class ItemPalette;
class KConfig;
class KAction;
class KRecentFilesAction;
class QItemSelection;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

public slots:
    bool newFile();
    bool openFile(const KUrl& url = KUrl(), const KUrl& startUrl = KUrl());
    bool saveFileAs(const KUrl& url = KUrl(), const KUrl& startUrl = KUrl());
    bool saveFile();

    void openExample();
    void openLocalExample();
    void uploadExample();
    void downloadExamples();

    void configureStep();

    void simulationStartStop();
    void simulationStart();
    void simulationStop();
    void simulationStopped(int result);

protected slots:
    void updateCaption();
    void undoTextChanged(const QString& undoText);
    void redoTextChanged(const QString& redoText);
    void worldSelectionChanged(const QItemSelection&, const QItemSelection&);

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
    bool queryClose();
    bool maybeSave();

protected:
    WorldModel*         worldModel;
    WorldBrowser*       worldBrowser;
    PropertiesBrowser*  propertiesBrowser;
    InfoBrowser*        infoBrowser;
    ItemPalette*        itemPalette;
    WorldScene*         worldScene;
    WorldGraphicsView*  worldGraphicsView;

    KAction* actionSimulation;
    KAction* actionSimulationStart;
    KAction* actionSimulationStop;

    KAction* actionUndo;
    KAction* actionRedo;
    KAction* actionDelete;

    KRecentFilesAction* actionRecentFiles;

    KUrl currentFileUrl;
};

#endif

