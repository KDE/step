/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/** \file gravitation.h
 *  \brief GravitationForce and WeightForce classes
 */

#ifndef STEPCORE_GRAVITATION_H
#define STEPCORE_GRAVITATION_H

#include "world.h"
#include "object.h"
#include "constants.h"

namespace StepCore
{

class GravitationForce;
class WeightForce;

/** \ingroup errors
 *  \brief Errors object for GravitationForce
 */
class GravitationForceErrors: public ObjectErrors
{
    STEPCORE_OBJECT(GravitationForceErrors)

public:
    /** Constructs GravitationForceErrors */
    explicit GravitationForceErrors(Item* owner = nullptr)
        : ObjectErrors(owner), _gravitationConstVariance(0) {}

    /** Get owner as GravitationForce */
    GravitationForce* gravitationForce() const;

    /** Get gravitationConst variance */
    double gravitationConstVariance() const { return _gravitationConstVariance; }
    /** Set gravitationConst variance */
    void   setGravitationConstVariance(double gravitationConstVariance) {
        _gravitationConstVariance = gravitationConstVariance; }

protected:
    double _gravitationConstVariance;
    friend class GravitationForce;
};

/** \ingroup forces
 *  \brief Newton gravitational force.
 *
 *  The force acts between pairs of massive bodies (currently only Particle)
 *  and equals:
 *  \f[
 *      \overrightarrow{f} = G \frac{m_1 m_2 \overrightarrow{r}}
 *                                  {|\overrightarrow{r}|^3}
 *  \f]
 *  where:\n
 *  \f$G\f$ is GravitationForce::gravitationConst\n
 *  \f$m_1\f$ and \f$m_2\f$ is Particle::mass of first and second body\n
 *  \f$\overrightarrow{r}\f$ is difference of Particle::position
 *  of the first and second body
 *
 *  \todo Add interface for massive bodies, support bodies with
 *        distributed mass
 */
class GravitationForce : public Force
{
    STEPCORE_OBJECT(GravitationForce)

public:
    /** Constructs GravitationForce */
    explicit GravitationForce(double gravitationConst = Constants::Gravitational);

    void calcForce(bool calcVariances) override;

    /** Get gravitational constant */
    double gravitationConst() const { return _gravitationConst; }
    /** Set gravitational constant */
    void   setGravitationConst(double gravitationConst) { _gravitationConst = gravitationConst; }

    /** Get (and possibly create) GravitationForceErrors object */
    GravitationForceErrors* gravitationForceErrors() {
        return static_cast<GravitationForceErrors*>(objectErrors()); }

protected:
    ObjectErrors* createObjectErrors() override { return new GravitationForceErrors(this); }

    double _gravitationConst;
};

/** \ingroup errors
 *  \brief Errors object for WeightForce
 */
class WeightForceErrors: public ObjectErrors
{
    STEPCORE_OBJECT(WeightForceErrors)

public:
    /** Constructs WeightForceErrors */
    explicit WeightForceErrors(Item* owner = nullptr)
        : ObjectErrors(owner), _weightConstVariance(0) {}

    /** Get owner as WeightForce */
    WeightForce* weightForce() const;

    /** Get weightConst variance */
    double weightConstVariance() const { return _weightConstVariance; }
    /** Set weightConst variance */
    void   setWeightConstVariance(double weightConstVariance) {
        _weightConstVariance = weightConstVariance; }

protected:
    double _weightConstVariance;
    friend class WeightForce;
};

/** \ingroup forces
 *  \brief Weight force (constant gravitational force).
 *
 *  The force acts between on massive bodies (currently only Particle)
 *  and equals:
 *  \f[
 *      \overrightarrow{f} = \left( \begin{array}{c} 0 \\ mg \end{array} \right)
 *  \f]
 *  where:\n
 *  \f$m\f$ is Particle::mass\n
 *  \f$g\f$ is WeightForce::weightConst
 *
 *  \todo Add interface for massive bodies, support bodies with distributed mass
 */
class WeightForce : public Force
{
    STEPCORE_OBJECT(WeightForce)

public:
    /** Constructs WeightForce */
    explicit WeightForce(double weightConst = Constants::WeightAccel);

    void calcForce(bool calcVariances) override;

    /** Get weight constant */
    double weightConst() const { return _weightConst; }
    /** Set weight constant */
    void   setWeightConst(double weightConst) { _weightConst = weightConst; }

    /** Get (and possibly create) WeightForceErrors object */
    WeightForceErrors* weightForceErrors() {
        return static_cast<WeightForceErrors*>(objectErrors()); }

protected:
    ObjectErrors* createObjectErrors() override { return new WeightForceErrors(this); }

    double _weightConst;
};

} // namespace StepCore

#endif

