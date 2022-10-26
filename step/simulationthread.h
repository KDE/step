/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SIMULATIONTHREAD_H
#define SIMULATIONTHREAD_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

namespace StepCore {
    class World;
}

/**
 * Simulation thread only changes properties of items,
 * not their count or addresses, so locking is required for
 *  - any writes
 *  - reads of item properties
 *
 * @author Vladimir Kuznetsov
 */
class SimulationThread: public QThread
{
    Q_OBJECT

public:
    explicit SimulationThread(StepCore::World** world)
        : _world(world), _stopThread(0), _delta(0) {}
    ~SimulationThread();

    void run() override;
    void doWorldEvolve(double delta);
    QMutex* mutex() { return &_mutex; }

signals:
    void worldEvolveDone(int result);

protected:
    StepCore::World** _world;
    bool              _stopThread;
    double            _delta;
    QMutex            _mutex;
    QWaitCondition    _waitCondition;
};

#endif // SIMULATIONTHREAD_H
