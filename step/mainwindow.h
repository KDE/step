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
class UndoBrowser;
class ItemPalette;

class KConfig;
class KAction;
class KToolBarPopupAction;
class KRecentFilesAction;

class QItemSelection;
class QActionGroup;

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

    void openTutorial();
    void openExample();
    void openLocalExample();
    void uploadExample();
    void downloadExamples();

    void configureStep();

    void simulationStartStop();
    void simulationStart();
    void simulationStop();
    void simulationStopped(int result);

    void setRunSpeed(int);

    void setFullSpeed()     { setRunSpeed(0); }
    void setSlowSpeed()     { setRunSpeed(1); }
    void setSlowerSpeed()   { setRunSpeed(2); }
    void setSlowestSpeed()  { setRunSpeed(3); }
    void setStepSpeed()     { setRunSpeed(4); }


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
    UndoBrowser*        undoBrowser;
    ItemPalette*        itemPalette;
    WorldScene*         worldScene;
    WorldGraphicsView*  worldGraphicsView;

    KAction* actionUndo;
    KAction* actionRedo;
    KAction* actionDelete;

    KRecentFilesAction* actionRecentFiles;

    KUrl currentFileUrl;

    //The following members are needed for the setting of the timeScale
    int runSpeed;
    KToolBarPopupAction *runSpeedAction;
    KAction *fullSpeedAct;
    KAction *slowSpeedAct;
    KAction *slowerSpeedAct;
    KAction *slowestSpeedAct;
    KAction *stepSpeedAct;
    QActionGroup *runSpeedGroup;

};

#endif

