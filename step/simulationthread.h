#ifndef SIMULATIONTHREAD_H
#define SIMULATIONTHREAD_H
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

    void run() Q_DECL_OVERRIDE;
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
