/* This file is part of StepCore library.
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

/** \file spring.h
 *  \brief Spring class
 */

#ifndef STEPCORE_SPRING_H
#define STEPCORE_SPRING_H

#include "world.h"
#include "object.h"
#include "particle.h"

#include <QString>

namespace StepCore
{

/** \brief Helper class for forces that act between two selected bodies
 *
 * \todo create force.h for such things; MultiPairForce is also good idea
 */
class PairForce
{
public:
    /** Constructs PairForce */
    PairForce(Body* bodyPtr1 = 0, Body* bodyPtr2 = 0)
            : _bodyPtr1(bodyPtr1), _bodyPtr2(bodyPtr2) {}

    /** Get pointer to the first body */
    Body* bodyPtr1() { return _bodyPtr1; }
    /** Get pointer to the second body */
    Body* bodyPtr2() { return _bodyPtr2; }

#ifdef STEPCORE_WITH_QT
#endif

protected:
    Body* _bodyPtr1;
    Body* _bodyPtr2;
};

/** \ingroup forces
 *  \brief Massless spring
 *
 *  Massless spring is the force between two selected bodies which equals:
 *  \f[
 *      \overrightarrow{f} = k (\overrightarrow{r} - \overrightarrow{r_0})
 *  \f]
 *  where:\n
 *  \f$k\f$ is Spring::stiffness\n
 *  \f$\overrightarrow{r}\f$ is difference of Particle::position
 *  of the first and second body\n
 *  \f$\overrightarrow{r_0}\f$ is Spring::restLength
 *  
 *  \todo how to move setBody1() and setBody2() to PairForce ?
 */
class Spring: public Item, public Force, public PairForce
{
    //Q_OBJECT
    STEPCORE_OBJECT(Spring)

    /** Rest length of the spring */
    //Q_PROPERTY(double restLength READ restLength WRITE setRestLength)
    /** Current length of the spring */
    //Q_PROPERTY(double length READ length STORED false)
    /** Stiffness of the spring */
    //Q_PROPERTY(double stiffness READ stiffness WRITE setStiffness)
    /** Name of the first connected body */
    //Q_PROPERTY(QString body1 READ body1 WRITE setBody1)
    /** Name of the second connected body */
    //Q_PROPERTY(QString body2 READ body2 WRITE setBody2)

public:
    /** Constructs Spring */
    Spring(double restLength = 1, double stiffness = 1,
                Body* bodyPtr1 = 0, Body* bodyPtr2 = 0);

    void calcForce();

    /** Get rest length of the spring */
    double restLength() const { return _restLength; }
    /** Set rest length of the spring */
    void   setRestLength(double restLength) { _restLength = restLength; }

    /** Get current length of the spring */
    double length() const;

    /** Get stiffness of the spring */
    double stiffness() const { return _stiffness; }
    /** Set stiffness of the spring */
    void   setStiffness(double stiffness) { _stiffness = stiffness; }

    /** Set pointer to the first connected body */
    void setBodyPtr1(Body* bodyPtr1) { _bodyPtr1 = dynamic_cast<Particle*>(bodyPtr1); }
    /** Set pointer to the second connected body */
    void setBodyPtr2(Body* bodyPtr2) { _bodyPtr2 = dynamic_cast<Particle*>(bodyPtr2); }

#ifdef STEPCORE_WITH_QT
    /** Set first connected body by name */
    void setBody1(const QString& body1);
    /** Set second connected body by name */
    void setBody2(const QString& body2);
    /** Get name of the first connected body */
    QString body1() const { return _bodyPtr1 ? dynamic_cast<Item*>(_bodyPtr1)->name() : QString(); }
    /** Get name of the second connected body */
    QString body2() const { return _bodyPtr2 ? dynamic_cast<Item*>(_bodyPtr2)->name() : QString(); }
#endif

    void removeItem(Item* item);

protected:
    double _restLength;
    double _stiffness;
};

} // namespace StepCore

#endif

