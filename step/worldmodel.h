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

namespace StepCore {
    class World;
    class Item;
    class Solver;
}

class QItemSelectionModel;
class WorldFactory;

class WorldModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    enum { ObjectRole = Qt::UserRole, NewGraphicsItemRole };

    WorldModel(QObject* parent = 0);
    ~WorldModel();
    void clearWorld();

    QItemSelectionModel* selectionModel() const { return _selectionModel; }
    WorldFactory* worldFactory() const { return _worldFactory; }

    // QModelIndex quick-access functions
    QModelIndex worldIndex() const;
    QModelIndex solverIndex() const;
    QModelIndex itemIndex(int n) const;
    QModelIndex objectIndex(QObject* obj) const;
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

    // This used as notification only, actual editing is done outside
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    // Add/remove/set functions
    void addItem(StepCore::Item* item);
    void deleteItem(StepCore::Item* item);

    void setSolver(StepCore::Solver* solver);

    // Evolve
    bool doWorldEvolve(double delta);

    // Save/load
    bool saveXml(QIODevice* device);
    bool loadXml(QIODevice* device);
    QString errorString() const { return _errorString; }

    // Helper functions
    // XXX: use deeper tree to include properties ?
    QString variantToString(const QVariant& variant) const;
    QVariant stringToVariant(int typeId, const QString& string) const;

signals:
    void worldChanged(bool modified);

protected:
    void resetWorld();
    void emitChanged();

protected:
    StepCore::World* _world;
    QItemSelectionModel* _selectionModel;
    WorldFactory* _worldFactory;
    QString _errorString;
};

#endif

