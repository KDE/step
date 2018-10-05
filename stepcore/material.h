/* This file is part of StepCore library.
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

/** \file material.h
 *  \brief Material is what an object is made from. It has a color, density and some other physical properties.
 */

#ifndef STEPCORE_MATERIAL_H
#define STEPCORE_MATERIAL_H


#include <vector> // XXX: Replace if Qt is enabled.

#include <QString>

#include "types.h"


namespace StepCore
{


/** \ingroup materials
 *  \brief Interface for bodies
 *
 *  Material is anything that has dynamic variables that require ODE integration
 */
class Material
{
  //STEPCORE_OBJECT(Material)

public:
    explicit Material(const QString& name = QString())
        : _name(name)
        , _color(0xff000000)
        , _density(1.0)
    {}
    virtual ~Material() {}

    Material &operator=(const Material &mtrl);

    /** Get the name of the material. */
    QString  name() const  { return _name; }
    /** Set the name of the material. */
    void setName(const QString &name)  { _name = name; }

    /** Get the color of the material. */
    Color  color() const  { return _color; }
    /** Set the color of the material. */
    void setColor(const Color &color)  { _color = color; }

    /** Get the density of the material. */
    double density() const  { return _density; }
    /** Set the density of the material. */
    void setDensity(const double density)  { _density = density; }


private:
    QString  _name;

    Color    _color;
    double   _density;
    // ...more here?  Charge? other stuff?
};


/** List of pointers to Material */
typedef std::vector<Material*>  MaterialList;

extern Material GenericMaterial;


} // namespace StepCore


#endif
