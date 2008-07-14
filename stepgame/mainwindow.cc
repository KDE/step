/* This file is part of Game.
   Copyright (C) 2008 Aliona Kuznetsova <aliona.kuz@gmail.com>

   StepGame is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   StepGame is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with StepGame; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mainwindow.h"
#include "mainwindow.moc"

#include "worldmodel.h"
#include "worldscene.h"
#include "propertiesbrowser.h"
#include "infobrowser.h"
#include "undobrowser.h"
#include "itempalette.h"
#include "settings.h"
#include "unitscalc.h"

#include <stepcore/solver.h>
#include <stepcore/collisionsolver.h>

#include <KAction>
#include <KActionCollection>
#include <KStandardAction>
#include <KRecentFilesAction>
#include <KApplication>
#include <KMessageBox>
#include <KFileDialog>
#include <KTemporaryFile>
#include <KConfigDialog>
#include <KStandardDirs>
#include <KStatusBar>
#include <KLocale>
#include <KConfig>
#include <KToolBarPopupAction>

#include <KIO/NetAccess>
#include <KNS/Engine>

#include <QFile>
#include <QGraphicsView>
#include <QItemSelectionModel>
#include <QHBoxLayout>
#include <QMenu>

#include <cstdlib>
#include <ctime>

MainWindow::MainWindow()
{
    qsrand(time(NULL));
    std::srand(time(NULL));

    setObjectName("MainWindow");

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
    
    worldModel = new WorldModel(this);

    worldScene = new WorldScene(worldModel);

    worldGraphicsView = new WorldGraphicsView(worldScene, this);
    setCentralWidget(worldGraphicsView);
    
    worldModel->setMessageFrame(worldScene->messageFrame());

    connect(worldModel, SIGNAL(simulationStopped(int)), this, SLOT(simulationStopped(int)));
    connect(worldModel->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const  QItemSelection&)),
        this, SLOT(worldSelectionChanged(const QItemSelection&, const QItemSelection&)));
    setupActions();
    setupGUI();
    statusBar()->show();
    
    actionReset->setEnabled(false);
    worldModel->messageFrame()->closeMessage(simulationMessageId);

    newFile();

    simulationMessageId = 0;
}

MainWindow::~MainWindow()
{   
    KConfig* config = new KConfig("stepgamerc");
    actionRecentFiles->saveEntries(config->group("RecentFiles"));
    delete config;
}

void MainWindow::setupActions()
{
    /* File menu */
    //KStandardAction::openNew(this, SLOT(newFile()), actionCollection());
    KStandardAction::open(this, SLOT(openFile()), actionCollection());
    KStandardAction::save(this, SLOT(saveFile()), actionCollection());
    KStandardAction::saveAs(this, SLOT(saveFileAs()), actionCollection());
    KStandardAction::quit(this, SLOT(close()), actionCollection());
    actionRecentFiles = KStandardAction::openRecent(this, SLOT(openFile(const KUrl&)), actionCollection());

    KConfig* config = new KConfig("stepgamerc");
    actionRecentFiles->loadEntries(config->group("RecentFiles"));
    delete config;

    KAction* actionOpenLevel = actionCollection()->add<KAction>(
            "file_level_open", this, SLOT(openLevel()));
    actionOpenLevel->setText(i18n("&Open Level..."));
    actionOpenLevel->setIcon(KIcon("document-open"));

    KAction* actionOpenLocalLevel = actionCollection()->add<KAction>(
            "file_level_openlocal", this, SLOT(openLocalLevel()));
    actionOpenLocalLevel->setText(i18n("Open Down&loaded Level..."));
    actionOpenLocalLevel->setIcon(KIcon("document-open"));

    KAction* actionUploadLevel = actionCollection()->add<KAction>(
            "file_level_upload", this, SLOT(uploadLevel()));
    actionUploadLevel->setText(i18n("Share C&urrent Experiment..."));
    actionUploadLevel->setIcon(KIcon("get-hot-new-stuff"));

    KAction* actionDownloadLevels = actionCollection()->add<KAction>(
            "file_level_download", this, SLOT(downloadLevels()));
    actionDownloadLevels->setText(i18n("&Download New Experiments..."));
    actionDownloadLevels->setIcon(KIcon("get-hot-new-stuff"));
    
    actionReset = actionCollection()->add<KAction>("simulation_reset", this, SLOT(simulationReset()));
    actionReset->setText(i18n("&Reset"));
    actionReset->setIcon(KIcon("go-first"));

    /* Edit menu */
    actionRedo = KStandardAction::redo(worldModel->undoStack(), SLOT(redo()), actionCollection());
    actionUndo = KStandardAction::undo(worldModel->undoStack(), SLOT(undo()), actionCollection());
    actionRedo->setEnabled(false); actionUndo->setEnabled(false);
    actionRedo->setIconText(i18n("Redo")); actionUndo->setIconText(i18n("Undo"));
    connect(worldModel->undoStack(), SIGNAL(canRedoChanged(bool)), actionRedo, SLOT(setEnabled(bool)));
    connect(worldModel->undoStack(), SIGNAL(canUndoChanged(bool)), actionUndo, SLOT(setEnabled(bool)));
    connect(worldModel->undoStack(), SIGNAL(cleanChanged(bool)), this, SLOT(updateCaption()));
    connect(worldModel->undoStack(), SIGNAL(undoTextChanged(const QString&)),
            this, SLOT(undoTextChanged(const QString&)));
    connect(worldModel->undoStack(), SIGNAL(redoTextChanged(const QString&)),
            this, SLOT(redoTextChanged(const QString&)));
    connect(worldModel->undoStack(), SIGNAL(indexChanged (int)), this, SLOT(undoIndexChanged(int)));

    /* Simulation menu */
    // The run speed action group
    QActionGroup* runSpeedGroup = new QActionGroup(this);

    // The run action collection, this is used in the toolbar to create a dropdown menu on the run button
    runSpeedAction = new KToolBarPopupAction(KIcon("media-playback-start"), i18n("&Run"), this);
    connect(runSpeedAction, SIGNAL(triggered()), 
            this, SLOT(simulationStartStop()));
    QMenu* runSpeedActionMenu = runSpeedAction->menu();
    actionCollection()->addAction("run_speed", runSpeedAction);
    runSpeedActionMenu->setStatusTip(i18n("Execute the program"));
    runSpeedActionMenu->setWhatsThis(i18n("Run: Execute the program"));

    fullSpeedAct = new KAction(i18nc("@option:radio", "1x Speed"), this);
    actionCollection()->addAction("full_speed", fullSpeedAct );
    fullSpeedAct->setCheckable(true);
    fullSpeedAct->setChecked(true);
    connect(fullSpeedAct, SIGNAL(triggered()), this, SLOT(setFullSpeed()));
    runSpeedGroup->addAction(fullSpeedAct);
    runSpeedActionMenu->addAction(fullSpeedAct);
    
    slowSpeedAct = new KAction(i18nc("@option:radio choose the slow speed", "2x Speed"), this);
    actionCollection()->addAction("slow_speed", slowSpeedAct );
    slowSpeedAct->setCheckable(true);
    connect(slowSpeedAct, SIGNAL(triggered()), this, SLOT(setSlowSpeed()));
    runSpeedGroup->addAction(slowSpeedAct);
    runSpeedActionMenu->addAction(slowSpeedAct);

    slowerSpeedAct = new KAction(i18nc("@option:radio", "4x Speed"), this);
    actionCollection()->addAction("slower_speed", slowerSpeedAct );
    slowerSpeedAct->setCheckable(true);
    connect(slowerSpeedAct, SIGNAL(triggered()), this, SLOT(setSlowerSpeed()));
    runSpeedGroup->addAction(slowerSpeedAct);
    runSpeedActionMenu->addAction(slowerSpeedAct);

    slowestSpeedAct = new KAction(i18nc("@option:radio", "8x Speed"), this);
    actionCollection()->addAction("slowest_speed", slowestSpeedAct );
    slowestSpeedAct->setCheckable(true);
    connect(slowestSpeedAct, SIGNAL(triggered()), this, SLOT(setSlowestSpeed()));
    runSpeedGroup->addAction(slowestSpeedAct);
    runSpeedActionMenu->addAction(slowestSpeedAct);

    stepSpeedAct = new KAction(i18nc("@option:radio", "16x Speed"), this);
    actionCollection()->addAction("step_speed", stepSpeedAct );
    stepSpeedAct->setCheckable(true);
    connect(stepSpeedAct, SIGNAL(triggered()), this, SLOT(setStepGameSpeed()));
    runSpeedGroup->addAction(stepSpeedAct);
    runSpeedActionMenu->addAction(stepSpeedAct);

    simulationStopped(0);

    /* View menu */
    KStandardAction::actualSize(worldGraphicsView, SLOT(actualSize()), actionCollection());
    KStandardAction::fitToPage(worldGraphicsView, SLOT(fitToPage()), actionCollection());
    KStandardAction::zoomIn(worldGraphicsView, SLOT(zoomIn()), actionCollection());
    KStandardAction::zoomOut(worldGraphicsView, SLOT(zoomOut()), actionCollection());

    /* Settings menu */
//    KStandardAction::preferences(this, SLOT(configureStepGame()), actionCollection());

}

void MainWindow::updateCaption()
{
    QString shownName;
    if(currentFileUrl.isEmpty()) shownName = "untitled.step";
    else shownName = currentFileUrl.pathOrUrl(); //QFileInfo(currentFileName).fileName();
    setCaption(shownName, !worldModel->undoStack()->isClean());
}

bool MainWindow::queryClose()
{
    if(worldModel->isSimulationActive()) simulationStop();
    if(maybeSave()) {
        return true;
    } else {
        return false;
    }
}

bool MainWindow::newFile()
{
    if(worldModel->isSimulationActive()) simulationStop();
    if(!maybeSave()) return false;

    worldModel->clearWorld();
    worldGraphicsView->actualSize();
    worldGraphicsView->centerOn(0,0);
    currentFileUrl = KUrl();
    updateCaption();

    setFullSpeed(); // resetting the speed to the default speed of 1x
    return true;
}

bool MainWindow::openFile(const KUrl& url, const KUrl& startUrl)
{
    if(worldModel->isSimulationActive()) simulationStop();
    if(!maybeSave()) return false;

    KUrl fileUrl = url;
    if(fileUrl.isEmpty()) {
        fileUrl = KFileDialog::getOpenUrl(startUrl, "*.step|StepGame files (*.step)", this);
        if(fileUrl.isEmpty()) return false;
    }

    worldModel->clearWorld();
    newFile();

    QString tmpFileName;
    if(! KIO::NetAccess::download(fileUrl, tmpFileName, this) ) {
        KMessageBox::error(this, KIO::NetAccess::lastErrorString());
        return false;
    }

    QFile file(tmpFileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        KMessageBox::sorry(this, i18n("Cannot open file '%1'", tmpFileName));
        KIO::NetAccess::removeTempFile(tmpFileName);
        return false;
    }
    
    if(!worldModel->loadXml(&file)) {
        KMessageBox::sorry(this, i18n("Cannot parse file '%1': %2", fileUrl.pathOrUrl(),
                           worldModel->errorString()));
        KIO::NetAccess::removeTempFile(tmpFileName);
        return false;
    }

    KIO::NetAccess::removeTempFile(tmpFileName);

    worldGraphicsView->fitToPage();
    currentFileUrl = fileUrl;
    updateCaption();
    actionRecentFiles->addUrl(fileUrl);
 
    return true;
}

bool MainWindow::saveFileAs(const KUrl& url, const KUrl& startUrl)
{
    if(worldModel->isSimulationActive()) simulationStop();
    KUrl fileUrl = url;
    if(fileUrl.isEmpty()) {
        fileUrl = KFileDialog::getSaveUrl(startUrl.isEmpty() ? currentFileUrl : startUrl,
                                          "*.step|StepGame files (*.step)", this);
        if(fileUrl.isEmpty()) return false;
        else if(KIO::NetAccess::exists(fileUrl, KIO::NetAccess::DestinationSide, this)) {
            int ret = KMessageBox::warningContinueCancel(this,
                    i18n( "The file \"%1\" already exists. Do you wish to overwrite it?", fileUrl.pathOrUrl()),
                          i18n("Warning - StepGame"), KStandardGuiItem::overwrite());
            if(ret != KMessageBox::Continue) return false;
        }
    }

    bool local = fileUrl.isLocalFile();
    QFile* file;

    if(!local) {
        file = new KTemporaryFile();
        static_cast<KTemporaryFile*>(file)->setAutoRemove(true);
    } else {
        file = new QFile(fileUrl.path());
    }

    if(!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        KMessageBox::sorry(this, i18n("Cannot open file '%1'", file->fileName()));
        delete file;
        return false;
    }
    
    if(!worldModel->saveXml(file)) {
        KMessageBox::sorry(this, i18n("Cannot save file '%1': %2", fileUrl.pathOrUrl(),
                           worldModel->errorString()));
        delete file;
        return false;
    }

    if(!local) {
        if(!KIO::NetAccess::upload(file->fileName(), fileUrl, this)) {
            KMessageBox::error(this, KIO::NetAccess::lastErrorString());
            delete file;
            return false;
        }
    }

    delete file;

    worldModel->undoStack()->setClean();
    currentFileUrl = fileUrl;
    updateCaption();
    return true;
}

bool MainWindow::saveFile()
{
    if(worldModel->isSimulationActive()) simulationStop();
    return saveFileAs(currentFileUrl);
}

bool MainWindow::maybeSave()
{
    if(!worldModel->undoStack()->isClean()) {
        int ret = KMessageBox::warningYesNoCancel(this, 
                i18n("The experiment has been modified.\nDo you want to save your changes?"),
                     i18n("Warning - StepGame"), KStandardGuiItem::save(), KStandardGuiItem::discard());
        if (ret == KMessageBox::Yes) return saveFile();
        else if(ret == KMessageBox::Cancel) return false;
    }
    return true;
}

void MainWindow::openLevel()
{
    // XXX: need to be redone
    QStringList dirs = KGlobal::dirs()->findDirs("appdata", "levels");
    QString localDir = KStandardDirs::locateLocal("appdata", "");
    foreach(const QString &dir, dirs) {
        if(!dir.startsWith(localDir)) {
            openFile(KUrl(), dir);
            return;
        }
    }
}

void MainWindow::openLocalLevel()
{
    // XXX: need to be redone
    QString dir = KStandardDirs::locateLocal("appdata", "levels");
    if(dir.isEmpty()) return;
    KStandardDirs::makeDir(dir);
    openFile(KUrl(), dir);
}

void MainWindow::uploadLevel()
{
    KMessageBox::sorry(this, i18n("Uploading is still not implemented in kdelibs."),
                       i18n("Sorry - StepGame"));
    /*
    int ret = KMessageBox::questionYesNo(this,
    i18n("Do you want to upload current experiment to public web server ?"),
    i18n("Question - StepGame"));
    if(ret != KMessageBox::Yes) return;

    if(currentFileUrl.isEmpty() || !worldModel->undoStack()->isClean()) {
    ret = KMessageBox::warningContinueCancel(this,
    i18n("The experiment is not saved. You should it before uploading."),
    i18n("Warning - StepGame"), KStandardGuiItem::save(), KStandardGuiItem::cancel());
    if(ret != KMessageBox::Continue) return;
    if(!saveFile()) return;
}

    KNS::Engine::upload( currentFileUrl.url() );
    */
}

void MainWindow::downloadLevels()
{
    KNS::Entry::List entries = KNS::Engine::download();
}

void MainWindow::simulationStartStop()
{
    if(worldModel->isSimulationActive()) simulationStop();
    else simulationStart();
}

void MainWindow::simulationStart()
{
    runSpeedAction->setIconText(i18n("&Stop"));
    runSpeedAction->setIcon(KIcon("media-playback-stop"));
    worldGraphicsView->setInteractive(false);
    actionReset->setEnabled(true);

    actionUndo->setEnabled(false);
    worldModel->simulationStart();
    if(!simulationMessageId){
        simulationMessageId = worldModel->messageFrame()->showMessage(MessageFrame::Information, "Trying your solution");
    }
}

void MainWindow::simulationStopped(int result)
{
    runSpeedAction->setIconText(i18n("&Simulate"));
    runSpeedAction->setIcon(KIcon("media-playback-start"));

    if(result == StepCore::Solver::ToleranceError) {
        KMessageBox::sorry(this, i18n("Cannot finish this step because local error "
                "is greater than local tolerance.\n"
                        "Please check solver settings and try again."));
    } else if(result == StepCore::Solver::IntersectionDetected || 
              result == StepCore::Solver::CollisionDetected) {
                  KMessageBox::sorry(this, i18n("Cannot finish this step because there are collisions "
                          "which cannot be resolved automatically.\n"
                                  "Please move colliding objects appart and try again."));
    } else if(result != StepCore::Solver::OK) {
        KMessageBox::sorry(this, i18n("Cannot finish this step because of an unknown error."));
    }
    
    if(simulationReseting) {
        simulationReseting = false;
        while(worldModel->undoStack()->undoText().startsWith("Simulate")) {
            worldModel->undoStack()->undo();
        }
    } 
    
}

void MainWindow::simulationReset()
{
    simulationReseting = true;
    if(worldModel->isSimulationActive())
        simulationStop();
    else
        simulationStopped(0);
}

void MainWindow::simulationStop()
{
    worldModel->simulationStop();
}

void MainWindow::setRunSpeed(int speed)
{
    switch (speed) {
        case 0: fullSpeedAct->setChecked(true);    
        worldModel->world()->setTimeScale( 1.0);
        break;
        case 1: slowSpeedAct->setChecked(true);
        worldModel->world()->setTimeScale( 2.0);
        break;
        case 2: slowerSpeedAct->setChecked(true);
        worldModel->world()->setTimeScale( 4.0);
        break;
        case 3: slowestSpeedAct->setChecked(true);
        worldModel->world()->setTimeScale( 8.0);
        break;
        case 4: stepSpeedAct->setChecked(true);
        worldModel->world()->setTimeScale( 16.0);
        break;
    }
    runSpeed = speed;
}

void MainWindow::undoTextChanged(const QString& undoText)
{
    if(undoText.isEmpty()) actionUndo->setText(i18n("&Undo"));
    else actionUndo->setText(i18n("&Undo: %1", undoText));
}

void MainWindow::redoTextChanged(const QString& redoText)
{
    if(redoText.isEmpty()) actionRedo->setText(i18n("Re&do"));
    else actionRedo->setText(i18n("Re&do: %1", redoText));
}

void MainWindow::undoIndexChanged(int idx)
{
    if(! worldModel->undoStack()->undoText().startsWith("Simulate")){
        worldGraphicsView->setInteractive(true);
        actionReset->setEnabled(false);
	worldModel->messageFrame()->closeMessage(simulationMessageId);
	simulationMessageId = 0;
    } else {
        actionReset->setEnabled(true);
    }
}

void MainWindow::worldSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    foreach(const QModelIndex &index, worldModel->selectionModel()->selection().indexes()) {
        if(worldModel->item(index)) {
            return;
        }
    }
}

void MainWindow::configureStepGame()
{
    if(KConfigDialog::showDialog( "settings" )) return; 

    KConfigDialog* dialog = new KConfigDialog(this, "settings", Settings::self());

//    Ui::ConfigureStepGameGeneralWidget generalUi;
    QWidget* generalWidget = new QWidget(0);
    generalWidget->setObjectName("general");
//    generalUi.setupUi(generalWidget);
    dialog->addPage(generalWidget, i18n("General"), "general");

    connect(dialog, SIGNAL(settingsChanged(const QString&)),
            worldGraphicsView, SLOT(settingsChanged())); 
 
    dialog->show();
}