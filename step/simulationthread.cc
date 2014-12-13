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
