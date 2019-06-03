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

#include <QUrl>

#include <KXmlGuiWindow>

class WorldModel;
class WorldBrowser;
class WorldScene;
class WorldGraphicsView;
class PropertiesBrowser;
class InfoBrowser;
class UndoBrowser;
class ItemPalette;

class KToolBarPopupAction;
class KRecentFilesAction;

class QAction;
class QActionGroup;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

public slots:
    bool newFile();
    bool openFile(const QUrl& url = QUrl(), const QUrl& startUrl = QUrl());
    bool saveFileAs(const QUrl& url = QUrl(), const QUrl& startUrl = QUrl());
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
    void worldSelectionChanged();

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
    bool queryClose() Q_DECL_OVERRIDE;
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

    QAction* actionUndo;
    QAction* actionRedo;
    QAction* actionDelete;
    QAction* actionCut;
    QAction* actionCopy;
    QAction* actionPaste;

    KRecentFilesAction* actionRecentFiles;

    QUrl currentFileUrl;

    //The following members are needed for the setting of the timeScale
    int runSpeed;
    KToolBarPopupAction *runSpeedAction;
    QAction *fullSpeedAct;
    QAction *slowSpeedAct;
    QAction *slowerSpeedAct;
    QAction *slowestSpeedAct;
    QAction *stepSpeedAct;
    QActionGroup *runSpeedGroup;

};

#endif

