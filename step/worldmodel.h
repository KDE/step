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
#include <QTime>
#include <QUndoCommand>
#include <QVariant>

#include <KActionCollection>

#include <stepcore/world.h>

namespace StepCore {
    class Object;
    class World;
    class Item;
    class Solver;
    class CollisionSolver;
    class ConstraintSolver;
    class MetaProperty;
}

class QIODevice;
class QItemSelectionModel;
class QTimer;
class QMenu;
class QUndoStack;

class Clipboard;
class WorldFactory;
class CommandSimulate;
class SimulationThread;

/** \brief Central class that represents StepCore::World in Step */
class WorldModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    enum { FormattedNameRole = Qt::UserRole+1, ClassNameRole };
    enum FormatFlag { FormatEditable = 1, FormatHideUnits = 2 };
    enum UndoFlag { UndoNoMerge = 1 };

    Q_DECLARE_FLAGS(FormatFlags, FormatFlag)
    Q_DECLARE_FLAGS(UndoFlags, UndoFlag)

public:
    explicit WorldModel(QObject* parent = 0);
    ~WorldModel();

    /** Get QItemSelectionModel associated with this WorldModel */
    QItemSelectionModel* selectionModel() const { return _selectionModel; }
    /** Get WorldFactory associated with this WorldModel */
    const WorldFactory* worldFactory() const { return _worldFactory; }

    // QModelIndex quick-access functions
    QModelIndex worldIndex() const; ///< Get index of StepCore::World
    QModelIndex solverIndex() const; ///< Get index of StepCore::Solver
    QModelIndex collisionSolverIndex() const; ///< Get index of StepCore::CollisionSolver
    QModelIndex constraintSolverIndex() const; ///< Get index of StepCore::ConstraintSolver
    QModelIndex objectIndex(StepCore::Object* obj) const; ///< Get index of given object by pointer
    QModelIndex childItemIndex(int n, StepCore::ItemGroup* group = NULL) const; ///< Get index of n-th child of the group
    
    /** Get StepCore::Object by index. \note Never change returned object directly ! */
    StepCore::Object* object(const QModelIndex& index) const;
    /** Find StepCore::Object by name. \note Never change returned object directly ! */
    StepCore::Object* object(const QString& name) const { return _world->object(name); }

    /** Get StepCore::World. \note Never change returned object directly ! */
    StepCore::World* world() const { return _world; }
    /** Get StepCore::Solver. \note Never change returned object directly ! */
    StepCore::Solver* solver() const { return _world->solver(); }
    /** Get StepCore::CollisionSolver. \note Never change returned object directly ! */
    StepCore::CollisionSolver* collisionSolver() const { return _world->collisionSolver(); }
    /** Get StepCore::ConstraintSolver. \note Never change returned object directly ! */
    StepCore::ConstraintSolver* constraintSolver() const { return _world->constraintSolver(); }

    /** Get StepCore::Item by index. \note Never change returned object directly ! */
    StepCore::Item* item(const QModelIndex& index) const;

    /** Get n-th child of the world. \note Never change returned object directly ! */
    StepCore::Item* childItem(int n) const { return _world->items()[n]; }
    /** Get count of the children of the world */
    int childItemCount() const { return _world->items().size(); }

    /** QAbstractItemModel::data() function */
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    /** QAbstractItemModel::index() function */
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    /** QAbstractItemModel::parent() function */
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    /** QAbstractItemModel::rowCount() function */
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    /** QAbstractItemModel::columnCount() function */
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    // Add/remove/set functions
    /** Generates new unique item name */
    QString newItemName(const QString& className);
    StepCore::Item* createItem(const QString& className, StepCore::ItemGroup* parent = 0);
    /** Creates new item of className with unique name */
    StepCore::Item* newItem(const QString& className, StepCore::ItemGroup* parent = 0);
    /** Add already created item to the world */
    void addItem(StepCore::Item* item, StepCore::ItemGroup* parent = 0);
    /** Delete item from the world */
    void deleteItem(StepCore::Item* item);

    /** Create new solver of class named className and copy
     *  similar properties of the old solver into the new solver. */
    StepCore::Solver* newSolver(const QString& name);

    // Undo/redo helpers
    QUndoStack* undoStack() { return _undoStack; } ///< Get associated QUndoStack
    void pushCommand(QUndoCommand* command); ///< Push new undo command
    void beginMacro(const QString& text); ///< Begin undo macro
    void endMacro(); ///< End undo macro
    
    Clipboard* clipboard() const { return _clipboard; }

    // Property edit
    /** Modify object property.
     *  \param object object to modify
     *  \param property property to modify
     *  \param value new value for the property
     *  \param flags UndoFlags
     *
     *  \note Set flags to UndoNoMerge in order to disable merging this
     *        command with previous on undo history and avoid changing of command order */
    void setProperty(StepCore::Object* object, const StepCore::MetaProperty* property,
                        const QVariant& value, UndoFlags flags = {});

    /** Modify object property.
     *  \param object object to modify
     *  \param propertyName name of the property to modify
     *  \param value new value for the property
     *  \param flags UndoFlags
     *
     *  \note Set flags to UndoNoMerge in order to disable merging this
     *        command with previous on undo history and avoid changing of command order */
    void setProperty(StepCore::Object* object, const QString& propertyName,
                     const QVariant& value, UndoFlags flags = {}) {
        setProperty(object, object->metaObject()->property(propertyName), value, flags);
    }

    // Format property value for display or edit
    /** Format object name using standard formatting */
    QString formatName(const StepCore::Object* object) const;
    /** Format object and class name using standard formatting */
    QString formatNameFull(const StepCore::Object* object) const;
    /** Format property using standard formatting
     *  \param object object
     *  \param objectErrors associated StepCore::ObjecErrors (if required)
     *  \param property property
     *  \param flags Format flags */
    QString formatProperty(const StepCore::Object* object,
                           const StepCore::Object* objectErrors,
                           const StepCore::MetaProperty* property,
                           FormatFlags flags = {}) const;

    // Tooltip
    /** Generate standard tooltip for object given by index */
    QString createToolTip(const QModelIndex& index) const;

    // ContextMenu
    /** Generate standard context many for object given by index */
    QMenu* createContextMenu(const QModelIndex& index);

    // Save/load
    /** Clear the world (delete everything in it and reset all settings */
    void clearWorld();
    /** Save the world into device */
    bool saveXml(QIODevice* device);
    /** Load the world from device */
    bool loadXml(QIODevice* device);
    /** Get last error string */
    QString errorString() const { return _errorString; }

    // Names
    /** Generate unique object name */
    QString getUniqueName(const QString& className) const;
    /** Check is the name is unique */
    bool checkUniqueName(const QString& name) const;

    // Simulation
    /** Set FPS of simulation */
    void setSimulationFps(int simulationFps);
    /** Get FPS of simulation */
    int simulationFps() { return _simulationFps; }

    /** Return true if simulation is active */
    bool isSimulationActive();

    /** Pauses the simulation until control returns to event loop */
    void simulationPause();
    
    void setActions(KActionCollection* actions) { _actions = actions; }

public slots:
    void simulationStart(); ///< Start simulation
    void simulationStop();  ///< Stop simulation

    void cutSelectedItems();
    void copySelectedItems();
    void pasteItems();
    void deleteSelectedItems(); ///< Delete all selected items

protected slots:
    void simulationFrameBegin();
    void simulationFrameEnd(int result);
    void doEmitChanged();

signals:
    /** This signal is called when the world is changed
     *  \param dynamicOnly Indicated whether only dynamic variables was changed
     *  \note Dynamic variables are variables that can change during simulation,
     *        non-dynamic variables can change only by user action */
    void worldDataChanged(bool dynamicOnly);

    /** This signal is called when simulation is stopped.
     *  \param result simulation result (from StepCore::Solver) */
    void simulationStopped(int result);

protected:
    void resetWorld();
    void emitChanged(bool fullUpdate, bool recalcFn);
    void addCreatedItem(StepCore::Item* item, StepCore::ItemGroup* parent = 0);
    void removeCreatedItem(StepCore::Item* item);
    StepCore::Solver* swapSolver(StepCore::Solver* solver);
    QList<StepCore::Item*> selectedItems();

    // Only for UndoCommand* classes
    //void objectChanged(const StepCore::Object* object);

protected:
    StepCore::World* _world;
    QItemSelectionModel* _selectionModel;
    QUndoStack* _undoStack;
    Clipboard* _clipboard;
    const WorldFactory* _worldFactory;
    QString _errorString;

    QTimer* _updatingTimer;
    bool    _updatingFullUpdate;
    bool    _updatingRecalcFn;

    QTimer*           _simulationTimer;
    QTimer*           _simulationTimer0;
    int               _simulationFps;
    CommandSimulate*  _simulationCommand;
    SimulationThread* _simulationThread;

    int   _simulationFrames;
    QTime _simulationStartTime;

    bool _simulationFrameWaiting;
    bool _simulationFrameSkipped;
    bool _simulationStopping;
    bool _simulationPaused;
    
    KActionCollection* _actions;

    friend class CommandEditProperty;
    friend class CommandNewItem;
    friend class CommandSetSolver;
    friend class CommandSimulate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WorldModel::FormatFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(WorldModel::UndoFlags)

#endif

