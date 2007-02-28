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

    void setSolver(StepCore::Solver* solver);

    // Undo/redo helpers
    void pushCommand(QUndoCommand* command);
    void beginMacro(const QString& text);
    void endMacro();

    // Property edit
    void setProperty(StepCore::Object* object, const StepCore::MetaProperty* property,
                        const QVariant& value, bool merge = false);

    // Save/load
    void clearWorld();
    bool saveXml(QIODevice* device);
    bool loadXml(QIODevice* device);
    QString errorString() const { return _errorString; }

    // Undo/redo
    QUndoStack* undoStack() { return _undoStack; }

    // Names
    QString getUniqueName(QString className) const;
    bool checkUniqueName(QString name) const;

    void setSimulationFps(int simulationFps);
    int simulationFps() { return _simulationFps; }

    bool isSimulationActive();

public slots:
    void simulationStart();
    void simulationStop(bool success=true);

protected slots:
    void simulationFrame();

signals:
    void simulationStopped(bool success);

protected:
    void resetWorld();
    void emitChanged();
    void addItem(StepCore::Item* item);
    void removeItem(StepCore::Item* item);
    bool doWorldEvolve(double delta);

protected:
    StepCore::World* _world;
    QItemSelectionModel* _selectionModel;
    QUndoStack* _undoStack;
    const WorldFactory* _worldFactory;
    QString _errorString;

    QTimer* _simulationTimer;
    int     _simulationFps;
    CommandSimulate* _simulationCommand;

    friend class CommandEditProperty;
    friend class CommandNewItem;
    friend class CommandSimulate;
};

#endif

