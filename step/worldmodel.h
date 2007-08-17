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

#include <stepcore/world.h>

namespace StepCore {
    class Object;
    class World;
    class Item;
    class Solver;
    class CollisionSolver;
    class MetaProperty;
}

class QItemSelectionModel;
class QUndoStack;
class QTimer;
class QMenu;
class WorldFactory;
class CommandSimulate;
class SimulationThread;


class WorldModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    enum { ObjectNameRole = Qt::UserRole+1, ClassNameRole };

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
    QModelIndex childItemIndex(int n, StepCore::ItemGroup* group = NULL) const;
    
    // DO NOT change returned object directly: it breaks undo/redo
    StepCore::Object* object(const QModelIndex& index) const;
    StepCore::Object* object(const QString& name) const { return _world->object(name); }

    StepCore::World* world() const { return _world; }
    StepCore::Solver* solver() const { return _world->solver(); }
    StepCore::CollisionSolver* collisionSolver() const { return _world->collisionSolver(); }

    StepCore::Item* item(const QModelIndex& index) const;

    StepCore::Item* childItem(int n) const { return _world->items()[n]; }
    int childItemCount() const { return _world->items().size(); }

    // QAbstractItemModel functions
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    // Add/remove/set functions
    StepCore::Item* newItem(const QString& name, StepCore::ItemGroup* parent = 0);
    void addItem(StepCore::Item* item, StepCore::ItemGroup* parent = 0);
    void deleteItem(StepCore::Item* item);

    //void setSolver(StepCore::Solver* solver);
    StepCore::Solver* newSolver(const QString& name);

    // Undo/redo helpers
    QUndoStack* undoStack() { return _undoStack; }
    void pushCommand(QUndoCommand* command);
    void beginMacro(const QString& text);
    void endMacro();

    void beginUpdate() { ++_updating; }
    void endUpdate();

    // Property edit
    void setProperty(StepCore::Object* object, const StepCore::MetaProperty* property,
                            const QVariant& value, bool merge = true);

    // Tooltip
    QString createToolTip(const QModelIndex& index) const;

    // ContextMenu
    QMenu* createContextMenu(const QModelIndex& index);

    // Save/load
    void clearWorld();
    bool saveXml(QIODevice* device);
    bool loadXml(QIODevice* device);
    QString errorString() const { return _errorString; }

    // Names
    QString getUniqueName(QString className) const;
    bool checkUniqueName(const QString& name) const;

    // Simulation
    void setSimulationFps(int simulationFps);
    int simulationFps() { return _simulationFps; }

    bool isSimulationActive();

    // Pause simulation until control
    // returns to event loop
    void simulationPause();

public slots:
    void simulationStart();
    void simulationStop();

    void deleteSelectedItems();

protected slots:
    void simulationFrameBegin();
    void simulationFrameEnd(int result);

signals:
    void worldDataChanged(bool dynamicOnly);
    void simulationStopped(int result);

protected:
    void resetWorld();
    void emitChanged(bool dynamicOnly = true);
    void addCreatedItem(StepCore::Item* item, StepCore::ItemGroup* parent = 0);
    void removeCreatedItem(StepCore::Item* item);
    StepCore::Solver* swapSolver(StepCore::Solver* solver);

    // Only for UndoCommand* classes
    //void objectChanged(const StepCore::Object* object);

protected:
    StepCore::World* _world;
    QItemSelectionModel* _selectionModel;
    QUndoStack* _undoStack;
    const WorldFactory* _worldFactory;
    QString _errorString;

    int  _updating;
    bool _updatingDynamicOnly;

    QTimer*           _simulationTimer;
    int               _simulationFps;
    CommandSimulate*  _simulationCommand;
    SimulationThread* _simulationThread;

    bool _simulationFrameWaiting;
    bool _simulationFrameSkipped;
    bool _simulationStopping;
    bool _simulationPaused;

    friend class CommandEditProperty;
    friend class CommandNewItem;
    friend class CommandSetSolver;
    friend class CommandSimulate;
};

#endif

