/* This file is part of Step
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

   StepCore library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   StepCore library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with StepCore; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "test_worldcopy.h"
#include "stepcore/world.h"
#include "stepcore/solver.h"
#include "stepcore/types.h"
#include <QTest>

class WorldCopyTestItem: public StepCore::Body,
                         public StepCore::Force
{
    STEPCORE_OBJECT(WorldCopyTestItem)

public:
    WorldCopyTestItem(): _world1(NULL), _link(NULL) {}
    void worldItemRemoved(Item* item);
    void setWorld(StepCore::World* world);

    WorldCopyTestItem* link() const { return _link; }
    void setLink(WorldCopyTestItem* link) { _link = link; }

    StepCore::World* world1() const { return _world1; }

    virtual int  variablesCount() { return 0; }
    virtual void setVariables(const double* position, const double* velocity,
               const double* positionVariance, const double* velocityVariance) {}
    virtual void getVariables(double* position, double* velocity,
                     double* positionVariance, double* velocityVariance) {}
    virtual void addForce(const double* force, const double* forceVariance) {}
    virtual void resetForce(bool resetVariance) {}
    virtual void getAccelerations(double* acceleration, double* accelerationVariance) {}
    virtual void getInverseMass(StepCore::VectorXd* inverseMass,
                            StepCore::DynSparseRowMatrix* variance, int offset) {}

    virtual void calcForce(bool calcVariances) {}

private:
    StepCore::World* _world1;
    WorldCopyTestItem* _link;
};

class WorldCopyTestSolver: public StepCore::Solver
{
    STEPCORE_OBJECT(WorldCopyTestSolver)

public:
    virtual int doCalcFn(double* t, const StepCore::VectorXd* y, const StepCore::VectorXd* yvar = 0,
                            StepCore::VectorXd* f = 0, StepCore::VectorXd* fvar = 0) { return Solver::OK; }
    virtual int doEvolve(double* t, double t1, StepCore::VectorXd* y, StepCore::VectorXd* yvar) { return Solver::OK; }
};

STEPCORE_META_OBJECT(WorldCopyTestItem, "WorldCopyTestItem", "TestItem", 0,
    STEPCORE_SUPER_CLASS(StepCore::Item) STEPCORE_SUPER_CLASS(StepCore::Body) STEPCORE_SUPER_CLASS(StepCore::Force),)
STEPCORE_META_OBJECT(WorldCopyTestSolver, "WorldCopyTestSolver", "TestSolver", 0, STEPCORE_SUPER_CLASS(StepCore::Solver),)

void WorldCopyTestItem::worldItemRemoved(Item* item)
{
    if(item == _link) _link = 0;
}

void WorldCopyTestItem::setWorld(StepCore::World* world)
{
    _world1 = world;
    if(world == NULL) _link = NULL;
    else if(this->world() != NULL && _link != NULL) {
        _link = dynamic_cast<WorldCopyTestItem*>(
            world->items()[ this->world()->childItemIndex(dynamic_cast<const Item*>(_link)) ]);
    }
    StepCore::Item::setWorld(world);
}

void MainTest::testWorldCopy()
{
    /* Initialize and create some objects */
    StepCore::World* world = new StepCore::World();
    world->addItem(new WorldCopyTestItem());
    world->addItem(new WorldCopyTestItem());
    world->setSolver(new WorldCopyTestSolver());

    QCOMPARE( int(world->items().size()), 2 );
    QCOMPARE( int(world->bodies().size()), 2 );
    QCOMPARE( int(world->forces().size()), 2 );

    world->setName("world1");
    world->setTime(10);
    world->setTimeScale(20);
    world->items()[0]->setName("item0");
    world->items()[1]->setName("item1");
    world->solver()->setName("solver1");

    dynamic_cast<WorldCopyTestItem*>(world->items()[0])->setLink(
                    dynamic_cast<WorldCopyTestItem*>(world->items()[1]));

    /* Copy constructor */
    StepCore::World* world1 = new StepCore::World(*world);
    
    QCOMPARE(world1->name(), world->name());
    QCOMPARE(world1->time(), world->time());
    QCOMPARE(world1->timeScale(), world->timeScale());
    QCOMPARE(world1->items().size(), world->items().size());
    QCOMPARE(world1->bodies().size(), world->bodies().size());
    QCOMPARE(world1->forces().size(), world->forces().size());
    QCOMPARE(world1->items()[0]->name(), world->items()[0]->name());
    QCOMPARE(world1->items()[1]->name(), world->items()[1]->name());
    QCOMPARE(world1->solver()->name(), world->solver()->name());

    QVERIFY(world1->items()[0] != world->items()[0]);
    QVERIFY(world1->items()[1] != world->items()[1]);
    QVERIFY(world1->solver() != world->solver());

    QVERIFY(dynamic_cast<StepCore::Item*>(world1->bodies()[0]) == world1->items()[0]);
    QVERIFY(dynamic_cast<StepCore::Item*>(world1->bodies()[1]) == world1->items()[1]);
    QVERIFY(dynamic_cast<StepCore::Item*>(world1->forces()[0]) == world1->items()[0]);
    QVERIFY(dynamic_cast<StepCore::Item*>(world1->forces()[1]) == world1->items()[1]);

    QVERIFY(dynamic_cast<WorldCopyTestItem*>(world1->items()[0])->world1() == world1);
    QVERIFY(dynamic_cast<WorldCopyTestItem*>(world1->items()[1])->world1() == world1);
    QVERIFY(dynamic_cast<WorldCopyTestItem*>(world1->items()[0])->link() ==
                    dynamic_cast<WorldCopyTestItem*>(world1->items()[1]));

    /* Remove item */
    //world1->deleteItem(world1->items()[1]);
    //QVERIFY(world1->items().size() == 1);
    //QVERIFY(dynamic_cast<WorldCopyTestItem*>(world1->items()[0])->link() == NULL);
}

QTEST_MAIN(TestWorldCopy)
