/* This file is part of StepCore library.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
   Copyright (C) 2014 Inge Wallin        <inge@lysator.liu.se>

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

/** \file body.h
 *  \brief Contains the Body object.
 */

#ifndef STEPCORE_BODY_H
#define STEPCORE_BODY_H


#include <vector> // XXX: Replace if Qt is enabled.

#include "types.h"
#include "material.h"
#include "item.h"


namespace StepCore
{


class Material;
/** \ingroup bodies
 *  \brief Interface for bodies
 *
 *  Body is anything that has dynamic variables that require ODE integration
 */
class Body : public Item
{
    STEPCORE_OBJECT(Body)

public:
    explicit Body(const QString& name = QString())
        : Item(name)
        , _material(&GenericMaterial)
	, _variablesOffset(0)
    {}
    virtual ~Body() {}

    /** Get the material of the body. */
    Material *material() const { return _material; }

    /** Set material of the  body (FIXME: Must be enhanced with META_PROPERTY later) */
    void setMaterial(Material *mtrl) { _material = mtrl; }

    /** Get count of dynamic variables (not including velocities) */
    virtual int  variablesCount() = 0;

    /** Set positions, velocities and (possibly) its variances using values in arrays and
     *  also reset accelerations and its variances. Variances should only be copied
     *  and reset if positionVariance != NULL. */
    virtual void setVariables(const double* position, const double* velocity,
               const double* positionVariance, const double* velocityVariance) = 0;

    /** Copy positions, velocities and (possibly) its variances to arrays.
     *  Variances should only be copied if positionVariance != NULL. */
    virtual void getVariables(double* position, double* velocity,
                     double* positionVariance, double* velocityVariance) = 0;

    /** Add force and (possibly) its variance to force accumulator.
     *  \note This function is used only by generic constraints handling code,
     *        force objects should use body-specific functions. */
    virtual void addForce(const double* force, const double* forceVariance) = 0;

    /** Reset force accumulator and (possibly) its variance to zero.
     *  Variance should only be reset if resetVariance == true. */
    virtual void resetForce(bool resetVariance) = 0;

    /** Copy acceleration (forces left-multiplied by inverse mass)
     *  and (possibly) its variances to arrays.
     *  Variances should only be copied if accelerationVariance != NULL. */
    virtual void getAccelerations(double* acceleration, double* accelerationVariance) = 0;

    /** Get inverse mass and (possibly) its variance matrices.
     *  Variance should only be copied of variance != NULL. */
    virtual void getInverseMass(VectorXd* inverseMass,
                                DynSparseRowMatrix* variance, int offset) = 0;
    
    /** Offset of body's variables in global arrays
     *  (meaningless if the body is not a part of the world) */
    int variablesOffset() const { return _variablesOffset; }

private:
    friend class World;

    Material  *_material;

    /** \internal Set offset of body's variables in global arrays */
    void setVariablesOffset(int variablesOffset)
    {
        _variablesOffset = variablesOffset;
    }

    int _variablesOffset;
};


/** List of pointers to Body */
typedef std::vector<Body*>  BodyList;


} // namespace StepCore


#endif
