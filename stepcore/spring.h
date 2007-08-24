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
#include "rigidbody.h"
#include "vector.h"

#include <QString>
#include <cmath>

namespace StepCore
{

class Spring;

/** \ingroup errors
 *  \brief Errors object for Spring
 */
class SpringErrors: public ObjectErrors
{
    STEPCORE_OBJECT(SpringErrors)

public:
    /** Constructs SpringErrors */
    SpringErrors(Item* owner = NULL)
        : ObjectErrors(owner), _restLengthVariance(0), _stiffnessVariance(0),
          _localPosition1Variance(0), _localPosition2Variance(0) {}

    /** Get owner as String */
    Spring* spring() const;

    /** Get restLength variance */
    double restLengthVariance() const { return _restLengthVariance; }
    /** Set restLength variance */
    void   setRestLengthVariance(double restLengthVariance) {
        _restLengthVariance = restLengthVariance; }

    /** Get current length of the spring */
    double lengthVariance() const;

    /** Get stiffness variance */
    double stiffnessVariance() const { return _stiffnessVariance; }
    /** Set stiffness variance */
    void   setStiffnessVariance(double stiffnessVariance) {
        _stiffnessVariance = stiffnessVariance; }

    /** Get localPosition1 variance */
    Vector2d localPosition1Variance() const { return _localPosition1Variance; }
    /** Set localPosition1 variance */
    void setLocalPosition1Variance(const Vector2d& localPosition1Variance) {
        _localPosition1Variance = localPosition1Variance; }

    /** Get localPosition2 variance */
    Vector2d localPosition2Variance() const { return _localPosition2Variance; }
    /** Set localPosition2 variance */
    void setLocalPosition2Variance(const Vector2d& localPosition2Variance) {
        _localPosition2Variance = localPosition2Variance; }

    /** Get position1 variance */
    Vector2d position1Variance() const;

    /** Get position2 variance */
    Vector2d position2Variance() const;

    /** Get tension variance */
    double tensionVariance() const;

protected:
    double _restLengthVariance;
    double _stiffnessVariance;

    StepCore::Vector2d _localPosition1Variance;
    StepCore::Vector2d _localPosition2Variance;

    friend class Spring;
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
class Spring: public Item, public Force
{
    STEPCORE_OBJECT(Spring)

public:
    /** Constructs Spring */
    explicit Spring(double restLength = 0, double stiffness = 1,
                Body* bodyPtr1 = 0, Body* bodyPtr2 = 0);

    void calcForce(bool calcVariances);

    /** Get rest length of the spring */
    double restLength() const { return _restLength; }
    /** Set rest length of the spring */
    void   setRestLength(double restLength) { _restLength = restLength; }

    /** Get current length of the spring */
    double length() const { return (position2()-position1()).norm(); }

    /** Get stiffness of the spring */
    double stiffness() const { return _stiffness; }
    /** Set stiffness of the spring */
    void   setStiffness(double stiffness) { _stiffness = stiffness; }

    /** Get pointer to the first body */
    Body* bodyPtr1() { return _bodyPtr1; }
    /** Set pointer to the first connected body */
    void setBodyPtr1(Body* bodyPtr1);

    /** Get pointer to the second body */
    Body* bodyPtr2() { return _bodyPtr2; }
    /** Set pointer to the second connected body */
    void setBodyPtr2(Body* bodyPtr2);

    /** Set first connected body by name */
    void setBody1(const QString& body1) { setBodyPtr1(dynamic_cast<Body*>(world()->item(body1))); }
    /** Get name of the first connected body */
    QString body1() const { return _bodyPtr1 ? dynamic_cast<Item*>(_bodyPtr1)->name() : QString(); }

    /** Set second connected body by name */
    void setBody2(const QString& body2) { setBodyPtr2(dynamic_cast<Body*>(world()->item(body2))); }
    /** Get name of the second connected body */
    QString body2() const { return _bodyPtr2 ? dynamic_cast<Item*>(_bodyPtr2)->name() : QString(); }

    /** Local position of the first end of the spring on the body
     *  or in the world (if the end is not connected) */
    Vector2d localPosition1() const { return _localPosition1; }
    /** Set local position of the first end of the spring on the body
     *  or in the world (if the end is not connected) */
    void setLocalPosition1(const Vector2d& localPosition1) { _localPosition1 = localPosition1; }

    /** Local position of the second end of the spring on the body
     *  or in the world (if the end is not connected) */
    Vector2d localPosition2() const { return _localPosition2; }
    /** Set local position of the second end of the spring on the body
     *  or in the world (if the end is not connected) */
    void setLocalPosition2(const Vector2d& localPosition2) { _localPosition2 = localPosition2; }

    /** Position of the first end of the spring */
    Vector2d position1() const;
    /** Set position of the first end of the spring (will be ignored the end is connected) */
    //void setPosition1(const Vector2d& position1) { if(!_bodyPtr1) _position1 = position1; }

    /** Position of the second end of the spring */
    Vector2d position2() const;
    /** Set position of the second end of the spring (will be ignored the end is connected) */
    //void setPosition2(const Vector2d& position2) { if(!_bodyPtr2) _position2 = position2; }

    /** Tension force */
    double tension() const;

    void worldItemRemoved(Item* item);
    void setWorld(World* world);

    /** Get (and possibly create) SpringErrors object */
    SpringErrors* springErrors() { return static_cast<SpringErrors*>(objectErrors()); }

protected:
    ObjectErrors* createObjectErrors() { return new SpringErrors(this); }

    Body* _bodyPtr1;
    Body* _bodyPtr2;
    double _restLength;
    double _stiffness;
    StepCore::Vector2d _localPosition1;
    StepCore::Vector2d _localPosition2;
    //StepCore::Vector2d _position1;
    //StepCore::Vector2d _position2;
};

} // namespace StepCore

#endif

