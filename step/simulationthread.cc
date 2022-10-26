/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "simulationthread.h"

#include <stepcore/world.h>
#include <stepcore/solver.h>

void SimulationThread::run()
{
    _mutex.lock();
    forever {
        _waitCondition.wait(&_mutex);
        
        if(_stopThread) break;

        int result;
        if(! (*_world)->evolveAbort()) {
            //qDebug("begin doWorldEvolve() t=%#x", int(QThread::currentThread()));
            result = (*_world)->doEvolve(_delta);
            //qDebug("end doWorldEvolve()");
        } else {
            result = StepCore::Solver::Aborted;
        }

        emit worldEvolveDone(result);
    }
    _mutex.unlock();
}

void SimulationThread::doWorldEvolve(double delta)
{
    // XXX: ensure that previous frame was finished
    _mutex.lock();
    _delta = delta;
    _waitCondition.wakeOne();
    _mutex.unlock();
    //}
    /*
    _mutex->lock();
    qDebug("begin doWorldEvolve() t=%#x", int(QThread::currentThread()));
    int result = (*_world)->doEvolve(delta);
    qDebug("end doWorldEvolve()");
    _mutex->unlock();
    emit worldEvolveDone(result);
    */
}

SimulationThread::~SimulationThread()
{
    _stopThread = true;
    _waitCondition.wakeOne();
    wait();
}
