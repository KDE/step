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

#include "worldmodel.h"
#include "worldscene.h"
#include "worldbrowser.h"
#include "propertiesbrowser.h"
#include "itempalette.h"

#include <KAction>
#include <KActionCollection>
#include <KStandardAction>
#include <KApplication>
#include <KMessageBox>
#include <KFileDialog>
#include <KStatusBar>
#include <KLocale>

#include <QFile>
#include <QGraphicsView>

MainWindow::MainWindow()
{
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

    worldScene = new WorldScene(worldModel);
    worldGraphicsView = new WorldGraphicsView(worldScene, this);
    setCentralWidget(worldGraphicsView);

    QObject::connect(worldModel, SIGNAL(worldChanged(bool)), this, SLOT(setModified(bool)));
    QObject::connect(worldModel, SIGNAL(simulationStopped(bool)), this, SLOT(simulationStopped(bool)));
    QObject::connect(itemPalette, SIGNAL(beginAddItem(const QString&)),
                                    worldScene, SLOT(beginAddItem(const QString&)));
    QObject::connect(worldScene, SIGNAL(endAddItem(const QString&, bool)),
                                    itemPalette, SLOT(endAddItem(const QString&, bool)));

    setupActions();
    setupGUI();
    statusBar()->show();

    modified = false;
    newFile();
}

void MainWindow::setupActions()
{
    /* File menu */
    KStandardAction::openNew(this, SLOT(newFile()), actionCollection());
    KStandardAction::open(this, SLOT(openFile()), actionCollection());
    KStandardAction::save(this, SLOT(saveFile()), actionCollection());
    KStandardAction::saveAs(this, SLOT(saveFileAs()), actionCollection());
    KStandardAction::quit(this, SLOT(close()), actionCollection());

    /* Edit menu */
    actionRedo = KStandardAction::redo(worldModel->undoStack(), SLOT(redo()), actionCollection());
    actionUndo = KStandardAction::undo(worldModel->undoStack(), SLOT(undo()), actionCollection());
    actionRedo->setEnabled(false); actionUndo->setEnabled(false);
    connect(worldModel->undoStack(), SIGNAL(canRedoChanged(bool)), actionRedo, SLOT(setEnabled(bool)));
    connect(worldModel->undoStack(), SIGNAL(canUndoChanged(bool)), actionUndo, SLOT(setEnabled(bool)));

    /* Simulation menu */
    actionSimulation = static_cast<KAction*>(
        actionCollection()->addAction("simulation_start_stop", this, SLOT(simulationStartStop())));
    actionSimulationStart = static_cast<KAction*>(
        actionCollection()->addAction("simulation_start", this, SLOT(simulationStartStop())));
    actionSimulationStop = static_cast<KAction*>(
        actionCollection()->addAction("simulation_stop", this, SLOT(simulationStartStop())));

    actionSimulationStart->setText(i18n("&Start"));
    actionSimulationStart->setIcon(KIcon("player_play"));
    actionSimulationStop->setText(i18n("&Stop"));
    actionSimulationStop->setIcon(KIcon("player_stop"));

    simulationStop();

    /* View menu */
    KStandardAction::zoomIn(worldGraphicsView, SLOT(zoomIn()), actionCollection());
    KStandardAction::zoomOut(worldGraphicsView, SLOT(zoomOut()), actionCollection());
    KStandardAction::fitToPage(worldGraphicsView, SLOT(fitToPage()), actionCollection());
}

void MainWindow::updateCaption()
{
    QString shownName;
    if(currentFileName.isEmpty()) shownName = "untitled.step";
    else shownName = QFileInfo(currentFileName).fileName();
    setCaption(shownName, modified);
}

void MainWindow::setModified(bool modified)
{
    this->modified = modified;
    updateCaption();
}

bool MainWindow::queryClose()
{
    if(maybeSave()) {
        return true;
    } else {
        return false;
    }
}

bool MainWindow::newFile()
{
    if(!maybeSave()) return false;

    worldModel->clearWorld();
    worldGraphicsView->actualSize();
    worldGraphicsView->centerOn(0,0);
    currentFileName = QString();
    updateCaption();
    return true;
}

bool MainWindow::openFile(const QString& name)
{
    if(!maybeSave()) return false;

    QString fileName = name;
    if(fileName.isEmpty()) {
        fileName = KFileDialog::getOpenFileName(KUrl(), "*.step|Step files (*.step)", this);
        if(fileName.isEmpty()) return false;
    }

    modified = false;
    newFile();

    // TODO: KIO

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        KMessageBox::sorry(this, i18n("Can't open file '%1'").arg(fileName));
        return false;
    }
    
    if(!worldModel->loadXml(&file)) {
        KMessageBox::sorry(this, i18n("Can't parse file '%1': %2").
                        arg(fileName).arg(worldModel->errorString()));
        return false;
    }

    currentFileName = fileName;
    updateCaption();
    return true;
}

bool MainWindow::saveFileAs(const QString& name)
{
    QString fileName = name;
    if(fileName.isEmpty()) {
        fileName = KFileDialog::getSaveFileName(KUrl::fromPath(currentFileName),
                                             "*.step|Step files (*.step)", this);
        if(fileName.isEmpty()) return false;
    }

    // TODO: KIO (and replace currentFileName by currentFileURL)

    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        KMessageBox::sorry(this, i18n("Can't open file '%1'").arg(fileName));
        return false;
    }
    
    if(!worldModel->saveXml(&file)) {
        KMessageBox::sorry(this, i18n("Can't save file '%1': %2")
                        .arg(fileName).arg(worldModel->errorString()));
        return false;
    }

    currentFileName = fileName;
    updateCaption();
    return true;
}

bool MainWindow::saveFile()
{
    return saveFileAs(currentFileName);
}

bool MainWindow::maybeSave()
{
    if(modified) {
         int ret = KMessageBox::warningYesNoCancel(this, 
              i18n("The document has been modified.\nDo you want to save your changes?"),
              i18n("Warning - Step"), KStandardGuiItem::save(), KStandardGuiItem::discard());
         if (ret == KMessageBox::Yes) return saveFile();
         else if(ret == KMessageBox::Cancel) return false;
    }
    return true;
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
    actionSimulation->setText(i18n("&Stop"));
    actionSimulation->setIcon(KIcon("player_stop"));
    worldModel->simulationStart();
}

void MainWindow::simulationStopped(bool success)
{
    actionSimulationStart->setEnabled(true);
    actionSimulationStop->setEnabled(false);
    actionSimulation->setText(i18n("&Simulate"));
    actionSimulation->setIcon(KIcon("player_play"));
    if(!success) { // XXX: KMessageBox
        KMessageBox::sorry(this, i18n("Can't finish this step becouse local error "
               "is bigger then local tolerance. "
               "Please check solver settings and try again."));
    }
}

void MainWindow::simulationStop()
{
    worldModel->simulationStop();
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
            i18n("Can't finish this step becouse local error is bigger then local tolerance.<br />"
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
