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

#include "mainwindow.h"
#include "mainwindow.moc"

#include "ui_configure_step_general.h"

#include "worldmodel.h"
#include "worldscene.h"
#include "worldbrowser.h"
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

#include <KIO/NetAccess>
#include <KNS/Engine>

#include <QFile>
#include <QGraphicsView>
#include <QItemSelectionModel>
#include <QHBoxLayout>

#include <cstdlib>
#include <ctime>

MainWindow::MainWindow()
{
    qsrand(time(NULL));
    std::srand(time(NULL));

    // Load UnitCalc at startup
    UnitsCalc::self();

    setObjectName("MainWindow");

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    worldModel = new WorldModel(this);

    itemPalette = new ItemPalette(worldModel, this);
    itemPalette->setObjectName("itemPalette");
    addDockWidget(Qt::LeftDockWidgetArea, itemPalette);

    worldBrowser = new WorldBrowser(worldModel, this);
    worldBrowser->setObjectName("worldBrowser");
    addDockWidget(Qt::RightDockWidgetArea, worldBrowser);

    propertiesBrowser = new PropertiesBrowser(worldModel, this);
    propertiesBrowser->setObjectName("propertiesBrowser");
    addDockWidget(Qt::RightDockWidgetArea, propertiesBrowser);

    infoBrowser = new InfoBrowser(worldModel, this);
    infoBrowser->setObjectName("infoBrowser");
    addDockWidget(Qt::RightDockWidgetArea, infoBrowser);

    undoBrowser = new UndoBrowser(worldModel, this);
    undoBrowser->setObjectName("undoBrowser");
    addDockWidget(Qt::RightDockWidgetArea, undoBrowser);

    worldScene = new WorldScene(worldModel);
    worldGraphicsView = new WorldGraphicsView(worldScene, this);
    setCentralWidget(worldGraphicsView);

    connect(worldModel, SIGNAL(simulationStopped(int)), this, SLOT(simulationStopped(int)));
    connect(worldModel->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                                 this, SLOT(worldSelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(itemPalette, SIGNAL(beginAddItem(const QString&)),
                                 worldScene, SLOT(beginAddItem(const QString&)));
    connect(worldScene, SIGNAL(endAddItem(const QString&, bool)),
                                 itemPalette, SLOT(endAddItem(const QString&, bool)));
    connect(worldScene, SIGNAL(linkActivated(const KUrl&)),
                                 infoBrowser, SLOT(openUrl(const KUrl&)));
    

    setupActions();
    setupGUI();
    statusBar()->show();

    newFile();
}

MainWindow::~MainWindow()
{
    KConfig* config = new KConfig("steprc");
    actionRecentFiles->saveEntries(config->group("RecentFiles"));
    delete config;
}

void MainWindow::setupActions()
{
    /* File menu */
    KStandardAction::openNew(this, SLOT(newFile()), actionCollection());
    KStandardAction::open(this, SLOT(openFile()), actionCollection());
    KStandardAction::save(this, SLOT(saveFile()), actionCollection());
    KStandardAction::saveAs(this, SLOT(saveFileAs()), actionCollection());
    KStandardAction::quit(this, SLOT(close()), actionCollection());
    actionRecentFiles = KStandardAction::openRecent(this, SLOT(openFile(const KUrl&)), actionCollection());

    KConfig* config = new KConfig("steprc");
    actionRecentFiles->loadEntries(config->group("RecentFiles"));
    delete config;

    KAction* actionOpenExample = actionCollection()->add<KAction>(
                "file_example_open", this, SLOT(openExample()));
    actionOpenExample->setText(i18n("&Open example..."));
    actionOpenExample->setIcon(KIcon("document-open"));

    KAction* actionOpenLocalExample = actionCollection()->add<KAction>(
                "file_example_openlocal", this, SLOT(openLocalExample()));
    actionOpenLocalExample->setText(i18n("Open down&loaded example..."));
    actionOpenLocalExample->setIcon(KIcon("document-open"));

    KAction* actionUploadExample = actionCollection()->add<KAction>(
                "file_example_upload", this, SLOT(uploadExample()));
    actionUploadExample->setText(i18n("Share c&urrent experiment..."));
    actionUploadExample->setIcon(KIcon("get-hot-new-stuff"));

    KAction* actionDownloadExamples = actionCollection()->add<KAction>(
                "file_example_download", this, SLOT(downloadExamples()));
    actionDownloadExamples->setText(i18n("&Download new experiments..."));
    actionDownloadExamples->setIcon(KIcon("get-hot-new-stuff"));

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

    actionDelete = actionCollection()->add<KAction>("edit_delete", worldModel, SLOT(deleteSelectedItems()));
    actionDelete->setText(i18n("&Delete"));
    actionDelete->setIcon(KIcon("edit-delete"));
    actionDelete->setShortcut(KShortcut(Qt::CTRL+Qt::Key_Delete));
    actionDelete->setEnabled(false);

    /* Simulation menu */
    actionSimulationStart = actionCollection()->add<KAction>("simulation_start", this, SLOT(simulationStart()));
    actionSimulationStop = actionCollection()->add<KAction>("simulation_stop", this, SLOT(simulationStop()));
    actionSimulation = actionCollection()->add<KAction>("simulation_start_stop", this, SLOT(simulationStartStop()));
    actionSimulation->setText(i18n("Simulation start/stop"));

    actionSimulationStart->setText(i18n("&Start"));
    actionSimulationStart->setIcon(KIcon("media-playback-start"));

    actionSimulationStop->setText(i18n("S&top"));
    actionSimulationStop->setIcon(KIcon("media-playback-stop"));

    simulationStopped(0);

    /* View menu */
    KStandardAction::actualSize(worldGraphicsView, SLOT(actualSize()), actionCollection());
    KStandardAction::fitToPage(worldGraphicsView, SLOT(fitToPage()), actionCollection());
    KStandardAction::zoomIn(worldGraphicsView, SLOT(zoomIn()), actionCollection());
    KStandardAction::zoomOut(worldGraphicsView, SLOT(zoomOut()), actionCollection());

    /* Settings menu */
    KStandardAction::preferences(this, SLOT(configureStep()), actionCollection());
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
    undoBrowser->setEmptyLabel(i18n("<new file>"));
    undoBrowser->setCurrentFileUrl(currentFileUrl);
    return true;
}

bool MainWindow::openFile(const KUrl& url, const KUrl& startUrl)
{
    if(worldModel->isSimulationActive()) simulationStop();
    if(!maybeSave()) return false;

    KUrl fileUrl = url;
    if(fileUrl.isEmpty()) {
        fileUrl = KFileDialog::getOpenUrl(startUrl, "*.step|Step files (*.step)", this);
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
    undoBrowser->setEmptyLabel(i18n("<open file: %1>", fileUrl.fileName()));
    undoBrowser->setCurrentFileUrl(currentFileUrl);

    return true;
}

bool MainWindow::saveFileAs(const KUrl& url, const KUrl& startUrl)
{
    if(worldModel->isSimulationActive()) simulationStop();
    KUrl fileUrl = url;
    if(fileUrl.isEmpty()) {
        fileUrl = KFileDialog::getSaveUrl(startUrl.isEmpty() ? currentFileUrl : startUrl,
                                             "*.step|Step files (*.step)", this);
        if(fileUrl.isEmpty()) return false;
        else if(KIO::NetAccess::exists(fileUrl, KIO::NetAccess::DestinationSide, this)) {
            int ret = KMessageBox::warningContinueCancel(this,
                        i18n( "The file \"%1\" already exists. Do you wish to overwrite it?", fileUrl.pathOrUrl()),
                        i18n("Warning - Step"), KStandardGuiItem::overwrite());
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
    undoBrowser->setCurrentFileUrl(currentFileUrl);
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
              i18n("Warning - Step"), KStandardGuiItem::save(), KStandardGuiItem::discard());
         if (ret == KMessageBox::Yes) return saveFile();
         else if(ret == KMessageBox::Cancel) return false;
    }
    return true;
}

void MainWindow::openExample()
{
    // XXX: need to be redone
    QStringList dirs = KGlobal::dirs()->findDirs("appdata", "examples");
    QString localDir = KStandardDirs::locateLocal("appdata", "");
    foreach(QString dir, dirs) {
        if(!dir.startsWith(localDir)) {
            openFile(KUrl(), dir);
            return;
        }
    }
}

void MainWindow::openLocalExample()
{
    // XXX: need to be redone
    QString dir = KStandardDirs::locateLocal("appdata", "examples");
    if(dir.isEmpty()) return;
    KStandardDirs::makeDir(dir);
    openFile(KUrl(), dir);
}

void MainWindow::uploadExample()
{
    KMessageBox::sorry(this, i18n("Uploading is still not implemented in kdelibs."),
                        i18n("Sorry - Step"));
    /*
    int ret = KMessageBox::questionYesNo(this,
                i18n("Do you want to upload current experiment to public web server ?"),
                i18n("Question - Step"));
    if(ret != KMessageBox::Yes) return;

    if(currentFileUrl.isEmpty() || !worldModel->undoStack()->isClean()) {
        ret = KMessageBox::warningContinueCancel(this,
                i18n("The experiment is not saved. You should it before uploading."),
                i18n("Warning - Step"), KStandardGuiItem::save(), KStandardGuiItem::cancel());
        if(ret != KMessageBox::Continue) return;
        if(!saveFile()) return;
    }

    KNS::Engine::upload( currentFileUrl.url() );
    */
}

void MainWindow::downloadExamples()
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
    actionSimulationStart->setEnabled(false);
    actionSimulationStop->setEnabled(true);
    actionSimulation->setIconText(i18n("&Stop"));
    actionSimulation->setIcon(KIcon("media-playback-stop"));
    undoBrowser->setUndoEnabled(false);
    actionUndo->setEnabled(false);
    worldModel->simulationStart();
}

void MainWindow::simulationStopped(int result)
{
    actionSimulationStart->setEnabled(true);
    actionSimulationStop->setEnabled(false);
    actionSimulation->setIconText(i18n("&Simulate"));
    actionSimulation->setIcon(KIcon("media-playback-start"));
    undoBrowser->setUndoEnabled(true);
    if(result == StepCore::Solver::ToleranceError) {
        KMessageBox::sorry(this, i18n("Cannot finish this step because local error "
               "is greater than local tolerance.\n"
               "Please check solver settings and try again."));
    } else if(result == StepCore::Solver::IntersectionDetected || 
              result == StepCore::Solver::CollisionDetected) {
        KMessageBox::sorry(this, i18n("Cannot finish this step because there are collisions "
               "which cannot be resolved automatically.\n"
               "Please move colliding objects appart and try again."));
    } else if(result != StepCore::Solver::OK) {\
        KMessageBox::sorry(this, i18n("Cannot finish this step because of unknown error."));
    }
}

void MainWindow::simulationStop()
{
    worldModel->simulationStop();
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

void MainWindow::worldSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    foreach(QModelIndex index, worldModel->selectionModel()->selection().indexes()) {
        if(worldModel->item(index)) {
            actionDelete->setEnabled(true);
            return;
        }
    }
    actionDelete->setEnabled(false);
}

void MainWindow::configureStep()
{
    if(KConfigDialog::showDialog( "settings" )) return; 

    KConfigDialog* dialog = new KConfigDialog(this, "settings", Settings::self());

    Ui::ConfigureStepGeneralWidget generalUi;
    QWidget* generalWidget = new QWidget(0);
    generalWidget->setObjectName("general");
    generalUi.setupUi(generalWidget);
    dialog->addPage(generalWidget, i18n("General"), "general");

    connect(dialog, SIGNAL(settingsChanged(const QString&)),
                worldGraphicsView, SLOT(settingsChanged())); 
    connect(dialog, SIGNAL(settingsChanged(const QString&)),
                propertiesBrowser, SLOT(settingsChanged())); 

    dialog->show();
}

/*
void MainWindow::on_actionNew_triggered(bool checked)
{
    if(maybeSave()) newFile();
}

void MainWindow::on_actionOpen_triggered(bool checked)
{
    if(maybeSave()) openFile(QString());
}

void MainWindow::on_actionSave_triggered(bool checked)
{
    saveFileAs(currentFileName);
}

void MainWindow::on_actionSaveAs_triggered(bool checked)
{
    saveFile();
}

void MainWindow::on_actionStep_triggered(bool checked)
{
    if(!worldModel->doWorldEvolve(0.1))
        QMessageBox::warning(this, i18n("Step"), // XXX: retrieve error message from solver !
            i18n("Cannot finish this step becouse local error is bigger than local tolerance.<br />"
               "Please check solver settings and try again."));
}

void MainWindow::on_actionSimulation_triggered(bool checked)
{
    if(!simulationTimer->isActive()) {
        actionSimulation->setText(i18n("&Stop"));
        simulationTimer->start();
    } else {
        simulationTimer->stop();
        actionSimulation->setText(i18n("&Simulation"));
    }
}

void MainWindow::on_simulationTimer_timeout()
{
    worldModel->doWorldEvolve(1.0/FPS);
}

void MainWindow::on_actionAboutStep_triggered(bool checked)
{
    QMessageBox::about(this, i18n("About Step"),
             i18n("<center>The <b>Step</b> is an interactive physical simulator.<br /><br />"
                "Distributed under terms of the GNU GPL license.<br />"
                "(C) 2006-2007 Kuznetsov Vladimir.</center>"));
}
*/
