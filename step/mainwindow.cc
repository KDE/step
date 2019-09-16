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

#include "ui_configure_step_general.h"

#include "clipboard.h"
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

#include <QAction>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsView>
#include <QIcon>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QMenu>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTemporaryFile>

#include <KActionCollection>
#include <KConfig>
#include <KConfigDialog>
#include <KIO/CopyJob>
#include <KIO/Job>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNS3/DownloadDialog>
#include <KRecentFilesAction>
#include <KStandardAction>
#include <KToolBarPopupAction>

#include <cstdlib>
#include <ctime>

MainWindow::MainWindow()
{
    qsrand(time(NULL));
    std::srand(time(NULL));

    // Load UnitCalc at startup
    UnitsCalc::self();

    setObjectName(QStringLiteral("MainWindow"));

    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    worldModel = new WorldModel(this);
    worldModel->setActions(actionCollection());

    itemPalette = new ItemPalette(worldModel, this);
    itemPalette->setObjectName(QStringLiteral("itemPalette"));
    addDockWidget(Qt::LeftDockWidgetArea, itemPalette);

    worldBrowser = new WorldBrowser(worldModel, this);
    worldBrowser->setObjectName(QStringLiteral("worldBrowser"));
    addDockWidget(Qt::RightDockWidgetArea, worldBrowser);

    propertiesBrowser = new PropertiesBrowser(worldModel, this);
    propertiesBrowser->setObjectName(QStringLiteral("propertiesBrowser"));
    addDockWidget(Qt::RightDockWidgetArea, propertiesBrowser);

    infoBrowser = new InfoBrowser(worldModel, this);
    infoBrowser->setObjectName(QStringLiteral("infoBrowser"));
    addDockWidget(Qt::RightDockWidgetArea, infoBrowser);

    undoBrowser = new UndoBrowser(worldModel, this);
    undoBrowser->setObjectName(QStringLiteral("undoBrowser"));
    addDockWidget(Qt::RightDockWidgetArea, undoBrowser);

    worldScene = new WorldScene(worldModel, this);
    worldGraphicsView = new WorldGraphicsView(worldScene, this);
    setCentralWidget(worldGraphicsView);

    connect(worldModel, &WorldModel::simulationStopped, this, &MainWindow::simulationStopped);
    connect(worldModel->selectionModel(), &QItemSelectionModel::selectionChanged,
                                 this, &MainWindow::worldSelectionChanged);
    connect(itemPalette, &ItemPalette::beginAddItem,
	    worldScene,  &WorldScene::beginAddItem);
    connect(worldScene,  &WorldScene::endAddItem,
	    itemPalette, &ItemPalette::endAddItem);
    connect(worldScene,  SIGNAL(linkActivated(QUrl)),
	    infoBrowser, SLOT(openUrl(QUrl)));
    connect(worldScene,  &WorldScene::endAddItem,
            this,        &MainWindow::worldSelectionChanged);
    

    setupActions();
    setupGUI();
    statusBar()->show();

    newFile();
}

MainWindow::~MainWindow()
{
    disconnect(worldModel->undoStack(), &QUndoStack::cleanChanged, this, &MainWindow::updateCaption);

    KConfig* config = new KConfig(QStringLiteral("steprc"));
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
    actionRecentFiles = KStandardAction::openRecent(this, SLOT(openFile(QUrl)),
						    actionCollection());

    KConfig* config = new KConfig(QStringLiteral("steprc"));
    actionRecentFiles->loadEntries(config->group("RecentFiles"));
    delete config;

    QAction * actionOpenTutorial = actionCollection()->add<QAction>(
                QStringLiteral("file_tutorial_open"), this, SLOT(openTutorial()));
    actionOpenTutorial->setText(i18n("&Open Tutorial..."));
    actionOpenTutorial->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));

    QAction * actionOpenExample = actionCollection()->add<QAction>(
                QStringLiteral("file_example_open"), this, SLOT(openExample()));
    actionOpenExample->setText(i18n("&Open Example..."));
    actionOpenExample->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));

    QAction * actionOpenLocalExample = actionCollection()->add<QAction>(
                QStringLiteral("file_example_openlocal"), this, SLOT(openLocalExample()));
    actionOpenLocalExample->setText(i18n("Open Down&loaded Example..."));
    actionOpenLocalExample->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));

    QAction * actionUploadExample = actionCollection()->add<QAction>(
                QStringLiteral("file_example_upload"), this, SLOT(uploadExample()));
    actionUploadExample->setText(i18n("Share C&urrent Experiment..."));
    actionUploadExample->setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));

    QAction * actionDownloadExamples = actionCollection()->add<QAction>(
                QStringLiteral("file_example_download"), this, SLOT(downloadExamples()));
    actionDownloadExamples->setText(i18n("&Download New Experiments..."));
    actionDownloadExamples->setIcon(QIcon::fromTheme(QStringLiteral("get-hot-new-stuff")));

    /* Edit menu */
    actionRedo = KStandardAction::redo(worldModel->undoStack(), SLOT(redo()), actionCollection());
    actionUndo = KStandardAction::undo(worldModel->undoStack(), SLOT(undo()), actionCollection());
    actionRedo->setEnabled(false); actionUndo->setEnabled(false);
    actionRedo->setIconText(i18n("Redo")); actionUndo->setIconText(i18n("Undo"));
    connect(worldModel->undoStack(), &QUndoStack::canRedoChanged, actionRedo, &QAction::setEnabled);
    connect(worldModel->undoStack(), &QUndoStack::canUndoChanged, actionUndo, &QAction::setEnabled);
    connect(worldModel->undoStack(), &QUndoStack::cleanChanged, this, &MainWindow::updateCaption);
    connect(worldModel->undoStack(), &QUndoStack::undoTextChanged,
                                 this, &MainWindow::undoTextChanged);
    connect(worldModel->undoStack(), &QUndoStack::redoTextChanged,
                                 this, &MainWindow::redoTextChanged);
    
    actionCut = KStandardAction::cut(worldModel, SLOT(cutSelectedItems()),
                                     actionCollection());
    actionCopy = KStandardAction::copy(worldModel, SLOT(copySelectedItems()),
                                       actionCollection());
    actionPaste = KStandardAction::paste(worldModel, SLOT(pasteItems()),
                                         actionCollection());
    actionCut->setEnabled(false);
    actionCopy->setEnabled(false);
    actionPaste->setEnabled(worldModel->clipboard()->canPaste());
    connect(worldModel->clipboard(), &Clipboard::canPasteChanged,
            actionPaste, &QAction::setEnabled);

    actionDelete = actionCollection()->add<QAction>(QStringLiteral("edit_delete"), worldModel, SLOT(deleteSelectedItems()));
    actionDelete->setText(i18n("&Delete"));
    actionDelete->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    actionDelete->setEnabled(false);
    actionCollection()->setDefaultShortcut(actionDelete, QKeySequence(Qt::Key_Delete));

    /* Simulation menu */
    // The run speed action group
    QActionGroup* runSpeedGroup = new QActionGroup(this);

    // The run action collection, this is used in the toolbar to create a dropdown menu on the run button
    runSpeedAction = new KToolBarPopupAction(QIcon::fromTheme(QStringLiteral("media-playback-start")), i18n("&Run"), this);
    connect(runSpeedAction, &QAction::triggered, 
            this, &MainWindow::simulationStartStop);
    QMenu* runSpeedActionMenu = runSpeedAction->menu();
    actionCollection()->addAction(QStringLiteral("run_speed"), runSpeedAction);
    runSpeedActionMenu->setStatusTip(i18n("Execute the program"));
    runSpeedActionMenu->setWhatsThis(i18n("Run: Execute the program"));

    fullSpeedAct = new QAction(i18nc("@option:radio", "1x Speed"), this);
    actionCollection()->addAction(QStringLiteral("full_speed"), fullSpeedAct );
    fullSpeedAct->setCheckable(true);
    fullSpeedAct->setChecked(true);
    connect(fullSpeedAct, &QAction::triggered, this, &MainWindow::setFullSpeed);
    runSpeedGroup->addAction(fullSpeedAct);
    runSpeedActionMenu->addAction(fullSpeedAct);
	
    slowSpeedAct = new QAction(i18nc("@option:radio choose the slow speed", "2x Speed"), this);
    actionCollection()->addAction(QStringLiteral("slow_speed"), slowSpeedAct );
    slowSpeedAct->setCheckable(true);
    connect(slowSpeedAct, &QAction::triggered, this, &MainWindow::setSlowSpeed);
    runSpeedGroup->addAction(slowSpeedAct);
    runSpeedActionMenu->addAction(slowSpeedAct);

    slowerSpeedAct = new QAction(i18nc("@option:radio", "4x Speed"), this);
    actionCollection()->addAction(QStringLiteral("slower_speed"), slowerSpeedAct );
    slowerSpeedAct->setCheckable(true);
    connect(slowerSpeedAct, &QAction::triggered, this, &MainWindow::setSlowerSpeed);
    runSpeedGroup->addAction(slowerSpeedAct);
    runSpeedActionMenu->addAction(slowerSpeedAct);

    slowestSpeedAct = new QAction(i18nc("@option:radio", "8x Speed"), this);
    actionCollection()->addAction(QStringLiteral("slowest_speed"), slowestSpeedAct );
    slowestSpeedAct->setCheckable(true);
    connect(slowestSpeedAct, &QAction::triggered, this, &MainWindow::setSlowestSpeed);
    runSpeedGroup->addAction(slowestSpeedAct);
    runSpeedActionMenu->addAction(slowestSpeedAct);

    stepSpeedAct = new QAction(i18nc("@option:radio", "16x Speed"), this);
    actionCollection()->addAction(QStringLiteral("step_speed"), stepSpeedAct );
    stepSpeedAct->setCheckable(true);
    connect(stepSpeedAct, &QAction::triggered, this, &MainWindow::setStepSpeed);
    runSpeedGroup->addAction(stepSpeedAct);
    runSpeedActionMenu->addAction(stepSpeedAct);

    simulationStopped(0);

    /* View menu */
    KStandardAction::actualSize(worldGraphicsView, SLOT(actualSize()), actionCollection());
    KStandardAction::fitToPage(worldGraphicsView, SLOT(fitToPage()), actionCollection());
    KStandardAction::zoomIn(worldGraphicsView, SLOT(zoomIn()), actionCollection());
    KStandardAction::zoomOut(worldGraphicsView, SLOT(zoomOut()), actionCollection());

    /* Settings menu */
    KStandardAction::preferences(this, SLOT(configureStep()), actionCollection());

    /* Dock widgets */
    actionCollection()->addAction(QStringLiteral("toggle_palette_dock"), itemPalette->toggleViewAction());
    actionCollection()->addAction(QStringLiteral("toggle_world_dock"), worldBrowser->toggleViewAction());
    actionCollection()->addAction(QStringLiteral("toggle_properties_dock"), propertiesBrowser->toggleViewAction());
    actionCollection()->addAction(QStringLiteral("toggle_info_dock"), infoBrowser->toggleViewAction());
    actionCollection()->addAction(QStringLiteral("toggle_undo_dock"), undoBrowser->toggleViewAction());
}

void MainWindow::updateCaption()
{
    QString shownName;
    if (currentFileUrl.isEmpty())
	shownName = i18nc("filename", "untitled.step"); //Fixme if needed
    else
	shownName = currentFileUrl.url(QUrl::PreferLocalFile); //QFileInfo(currentFileName).fileName();
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
    currentFileUrl = QUrl();
    updateCaption();
    undoBrowser->setEmptyLabel(i18n("<new file>"));
    undoBrowser->setCurrentFileUrl(currentFileUrl);

    setFullSpeed(); // resetting the speed to the default speed of 1x
    return true;
}

bool MainWindow::openFile(const QUrl& url, const QUrl& startUrl)
{
    if(worldModel->isSimulationActive()) simulationStop();
    if(!maybeSave()) return false;

    QUrl fileUrl = url;
    if(fileUrl.isEmpty()) {
        fileUrl = QFileDialog::getOpenFileUrl(this, i18n("Open Step File"), startUrl, i18n("Step files (*.step)"));
        if(fileUrl.isEmpty()) return false;
    }

    worldModel->clearWorld();
    newFile();

    QFile file(fileUrl.path());
    
    if(!worldModel->loadXml(&file)) {
        KMessageBox::sorry(this, i18n("Cannot parse file '%1': %2", fileUrl.url(QUrl::PreferLocalFile),
                                                    worldModel->errorString()));
        return false;
    }

    worldGraphicsView->fitToPage();
    currentFileUrl = fileUrl;
    updateCaption();
    actionRecentFiles->addUrl(fileUrl);
    undoBrowser->setEmptyLabel(i18n("<open file: %1>", fileUrl.fileName()));
    undoBrowser->setCurrentFileUrl(currentFileUrl);

    return true;
}

bool MainWindow::saveFileAs(const QUrl& url, const QUrl& startUrl)
{
    if(worldModel->isSimulationActive()) simulationStop();
    QUrl fileUrl = url;
    if(fileUrl.isEmpty()) {
        fileUrl = QFileDialog::getSaveFileUrl(this, i18n("Save Step File"), startUrl.isEmpty() ? currentFileUrl : startUrl, i18n("Step files (*.step)"));
        if(fileUrl.isEmpty()) return false;
        }

    bool local = fileUrl.isLocalFile();
    QFile* file;

    if(!local) {
        QTemporaryFile *tempFile = new QTemporaryFile();
        tempFile->setAutoRemove(true);
	file = tempFile;
    } else {
        file = new QFile(fileUrl.path());
    }

    if(!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        KMessageBox::sorry(this, i18n("Cannot open file '%1'", file->fileName()));
        delete file;
        return false;
    }
    
    if(!worldModel->saveXml(file)) {
        KMessageBox::sorry(this, i18n("Cannot save file '%1': %2",
				      fileUrl.url(QUrl::PreferLocalFile),
				      worldModel->errorString()));
        delete file;
        return false;
    }

    if(!local) {
        KIO::FileCopyJob *job = KIO::file_copy(QUrl::fromLocalFile(file->fileName()), fileUrl, -1, KIO::Overwrite);
        KJobWidgets::setWindow(job, this);
        job->exec();
        if (job->error()) {
            KMessageBox::error(this, job->errorString());
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

void MainWindow::openTutorial()
{
    // XXX: need to be redone
    //qDebug() << "inside MainWindow::openTutorial()";
    QStringList dirs = QStandardPaths::locateAll(QStandardPaths::DataLocation, QStringLiteral("tutorials"), QStandardPaths::LocateDirectory);
    QString localDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/');
    foreach(const QString &dirName, dirs) {
        //qDebug() << "dirName: " << dirName;
        if(!dirName.startsWith(localDir)) {
	    openFile(QUrl(), QUrl::fromLocalFile(dirName));
            return;
        }
    }
}

void MainWindow::openExample()
{
    //qDebug() << "inside MainWindow::openExample()";
    // XXX: need to be redone
    QStringList dirs = QStandardPaths::locateAll(QStandardPaths::DataLocation, QStringLiteral("examples"), QStandardPaths::LocateDirectory);
    QString localDir = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/');
    foreach(const QString &dirName, dirs) {
        if(!dirName.startsWith(localDir)) {
            openFile(QUrl(), QUrl::fromLocalFile(dirName));
            return;
        }
    }
}

void MainWindow::openLocalExample()
{
    // XXX: need to be redone
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/examples";
    if(dir.isEmpty()) return;
    QDir::root().mkpath(dir);
    openFile(QUrl(), QUrl::fromLocalFile(dir));
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
    //qDebug() << "inside MainWindow::downloadExamples()";
    KNS3::DownloadDialog dialog(QStringLiteral("step.knsrc"), this);
    dialog.exec();
}

void MainWindow::simulationStartStop()
{
    if(worldModel->isSimulationActive()) simulationStop();
    else simulationStart();
}

void MainWindow::simulationStart()
{
    runSpeedAction->setIconText(i18n("&Stop"));
    runSpeedAction->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-stop")));

    undoBrowser->setUndoEnabled(false);
    actionUndo->setEnabled(false);
    worldModel->simulationStart();
}

void MainWindow::simulationStopped(int result)
{
    runSpeedAction->setIconText(i18n("&Simulate"));
    runSpeedAction->setIcon(QIcon::fromTheme(QStringLiteral("media-playback-start")));

    undoBrowser->setUndoEnabled(true);
    if(result == StepCore::Solver::ToleranceError) {
        KMessageBox::sorry(this, i18n("Cannot finish this step because local error "
               "is greater than local tolerance.\n"
               "Please check solver settings and try again."));
    } else if(result == StepCore::Solver::IntersectionDetected || 
              result == StepCore::Solver::CollisionDetected) {
        KMessageBox::sorry(this, i18n("Cannot finish this step because there are collisions "
               "which cannot be resolved automatically.\n"
               "Please move colliding objects apart and try again."));
    } else if(result != StepCore::Solver::OK) {
        KMessageBox::sorry(this, i18n("Cannot finish this step because of an unknown error."));
    }
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

void MainWindow::worldSelectionChanged()
{
    if (!worldScene->hasItemCreator()) {
        foreach (const QModelIndex &index,
                 worldModel->selectionModel()->selection().indexes()) {
            if (index != worldModel->worldIndex() && worldModel->item(index)) {
                actionDelete->setEnabled(true);
                actionCut->setEnabled(true);
                actionCopy->setEnabled(true);
                return;
            }
        }
    }
    actionDelete->setEnabled(false);
    actionCut->setEnabled(false);
    actionCopy->setEnabled(false);
}

void MainWindow::configureStep()
{
    if(KConfigDialog::showDialog( QStringLiteral("settings") )) return; 

    KConfigDialog* dialog = new KConfigDialog(this, QStringLiteral("settings"), Settings::self());

    Ui::ConfigureStepGeneralWidget generalUi;
    QWidget* generalWidget = new QWidget(0);
    generalWidget->setObjectName(QStringLiteral("general"));
    generalUi.setupUi(generalWidget);
    dialog->addPage(generalWidget, i18n("General"), QStringLiteral("step")); //shows the "step" icon, the "general" icon doesn't exist

    connect(dialog, &KConfigDialog::settingsChanged,
                worldGraphicsView, &WorldGraphicsView::settingsChanged); 
    connect(dialog, &KConfigDialog::settingsChanged,
                propertiesBrowser, &PropertiesBrowser::settingsChanged); 

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
            i18n("Cannot finish this step because local error is bigger than local tolerance.<br />"
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
