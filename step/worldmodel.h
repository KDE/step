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

#ifndef STEP_WORLDMODEL_H
#define STEP_WORLDMODEL_H

#include <QAbstractItemModel>
#include <QUndoCommand>
#include <QVariant>
#include <QThread>
#include <QMutex>

namespace StepCore {
    class Object;
    class World;
    class Item;
    class Solver;
    class MetaProperty;
}

class QItemSelectionModel;
class QUndoStack;
class QTimer;
class WorldFactory;
class CommandSimulate;

/* Simulation thread only changes properties of items,
 * not their count or addresses, so locking is required for
 *  - any writes
 *  - reads of item properties
 */
class SimulationThread: public QThread
{
    Q_OBJECT

public:
    SimulationThread(StepCore::World** world, QMutex* mutex)
        : _world(world), _mutex(mutex) {}
    void run() { exec(); }

public slots:
    void doWorldEvolve(double delta);

signals:
    void worldEvolveDone(int result);

protected:
    StepCore::World** _world;
    QMutex*           _mutex;
};

class WorldModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    WorldModel(QObject* parent = 0);
    ~WorldModel();

    QItemSelectionModel* selectionModel() const { return _selectionModel; }
    const WorldFactory* worldFactory() const { return _worldFactory; }

    // QModelIndex quick-access functions
    QModelIndex worldIndex() const;
    QModelIndex solverIndex() const;
    QModelIndex collisionSolverIndex() const;
    QModelIndex objectIndex(StepCore::Object* obj) const;
    QModelIndex itemIndex(int n) const;
    
    // DO NOT change returned object directly: it breaks undo/redo
    StepCore::Object* object(const QModelIndex& index) const;
    // Only for UndoCommand* classes
    void objectChanged(const StepCore::Object* object);

    StepCore::Item* item(const QModelIndex& index) const;
    StepCore::Item* item(int n) const { return item(itemIndex(n)); }
    int itemCount() const;

    // QAbstractItemModel functions
    // XXX: disallow external modifications - replace ObjectRole by PropertyRole
    //      or include properties in the tree
    // XXX: All modification should be done through setData
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    // Add/remove/set functions
    StepCore::Item* newItem(const QString& name);
    void deleteItem(StepCore::Item* item);

    //void setSolver(StepCore::Solver* solver);
    StepCore::Solver* newSolver(const QString& name);

    // Undo/redo helpers
    QUndoStack* undoStack() { return _undoStack; }
    void pushCommand(QUndoCommand* command);
    void beginMacro(const QString& text);
    void endMacro();

    void beginUpdate() { ++_updating; }
    void endUpdate() { if(!--_updating) objectChanged(NULL); }

    // Property edit
    void setProperty(StepCore::Object* object, const StepCore::MetaProperty* property,
                            const QVariant& value, bool merge = true);

    // Tooltip
    QString createToolTip(const StepCore::Object* object) const;

    // Save/load
    void clearWorld();
    bool saveXml(QIODevice* device);
    bool loadXml(QIODevice* device);
    QString errorString() const { return _errorString; }

    // Names
    QString getUniqueName(QString className) const;
    bool checkUniqueName(QString name) const;

    void setSimulationFps(int simulationFps);
    int simulationFps() { return _simulationFps; }

    bool isSimulationActive();

public slots:
    void simulationStart();
    void simulationStop();

    void deleteSelectedItems();

protected slots:
    void simulationFrameBegin();
    void simulationFrameEnd(int result);

signals:
    void simulationStopped(int result);
    void simulationDoFrame(double delta);

protected:
    void resetWorld();
    void emitChanged();
    void addItem(StepCore::Item* item);
    void removeItem(StepCore::Item* item);
    StepCore::Solver* swapSolver(StepCore::Solver* solver);

protected:
    StepCore::World* _world;
    QItemSelectionModel* _selectionModel;
    QUndoStack* _undoStack;
    const WorldFactory* _worldFactory;
    QString _errorString;

    int _updating;

    QTimer*           _simulationTimer;
    int               _simulationFps;
    CommandSimulate*  _simulationCommand;

    QMutex*           _simulationMutex;
    SimulationThread* _simulationThread;

    friend class CommandEditProperty;
    friend class CommandNewItem;
    friend class CommandSetSolver;
    friend class CommandSimulate;
};

#endif

