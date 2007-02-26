#include "maintest.h"

#include <stepcore/world.h>
#include <stepcore/solver.h>

class WorldCopyTestItem: public StepCore::Item
{
    STEPCORE_OBJECT(WorldCopyTestItem)

public:
    WorldCopyTestItem(): _world1(NULL), _link(NULL) {}
    void worldItemRemoved(Item* item);
    void setWorld(StepCore::World* world);

    WorldCopyTestItem* link() const { return _link; }
    void setLink(WorldCopyTestItem* link) { _link = link; }

    StepCore::World* world1() const { return _world1; }

private:
    StepCore::World* _world1;
    WorldCopyTestItem* _link;
};

class WorldCopyTestSolver: public StepCore::Solver
{
    STEPCORE_OBJECT(WorldCopyTestSolver)

public:
    void doCalcFn(double* t, double y[], double f[] = 0) {};
    bool doEvolve(double* t, double t1, double y[], double yerr[]) { return false; }
};

STEPCORE_META_OBJECT(WorldCopyTestItem,   "TestItem", 0, STEPCORE_SUPER_CLASS(StepCore::Item),)
STEPCORE_META_OBJECT(WorldCopyTestSolver, "TestSolver", 0, STEPCORE_SUPER_CLASS(StepCore::Solver),)

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
            world->items()[ this->world()->itemIndex(dynamic_cast<const Item*>(_link)) ]);
    }
    StepCore::Item::setWorld(world);
}

void MainTest::testWorldCopy()
{
    /* Initialize and create some objects */
    StepCore::World* world = new StepCore::World();
    world->addItem(new WorldCopyTestItem());
    world->addItem(new WorldCopyTestItem());
    world->setSolver(new WorldCopyTestSolver);

    world->setName("world1");
    world->setTime(10);
    world->items()[0]->setName("item0");
    world->items()[1]->setName("item1");
    world->solver()->setName("solver1");

    dynamic_cast<WorldCopyTestItem*>(world->items()[0])->setLink(
                    dynamic_cast<WorldCopyTestItem*>(world->items()[1]));

    /* Copy constructor */
    StepCore::World* world1 = new StepCore::World(*world);
    
    QCOMPARE(world1->name(), world->name());
    QCOMPARE(world1->time(), world->time());
    QCOMPARE(world1->items().size(), world->items().size());
    QCOMPARE(world1->items()[0]->name(), world->items()[0]->name());
    QCOMPARE(world1->items()[1]->name(), world->items()[1]->name());
    QCOMPARE(world1->solver()->name(), world->solver()->name());

    QVERIFY(world1->items()[0] != world->items()[0]);
    QVERIFY(world1->items()[1] != world->items()[1]);
    QVERIFY(world1->solver() != world->solver());

    QVERIFY(dynamic_cast<WorldCopyTestItem*>(world1->items()[0])->world1() == world1);
    QVERIFY(dynamic_cast<WorldCopyTestItem*>(world1->items()[1])->world1() == world1);
    QVERIFY(dynamic_cast<WorldCopyTestItem*>(world1->items()[0])->link() ==
                    dynamic_cast<WorldCopyTestItem*>(world1->items()[1]));

    /* Remove item */
    //world1->deleteItem(world1->items()[1]);
    //QVERIFY(world1->items().size() == 1);
    //QVERIFY(dynamic_cast<WorldCopyTestItem*>(world1->items()[0])->link() == NULL);
}

