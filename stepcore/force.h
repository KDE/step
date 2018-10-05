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

/** \file force.h
 *  \brief Contains the Force object.
 */

#ifndef STEPCORE_FORCE_H
#define STEPCORE_FORCE_H


#include <vector> // XXX: Replace if Qt is enabled.

#include "types.h"
#include "item.h"


namespace StepCore
{


/** \ingroup forces
 *  \brief Interface for forces
 *
 *  Force is anything that acts upon bodies changing derivatives of dynamic variables
 */
class Force : public Item
{
    STEPCORE_OBJECT(Force)

public:
    explicit Force(const QString& name = QString())
        : Item(name)
    {}
    virtual ~Force() {}

    /** Calculate force. Bodies can be accessed through
     * this->world()->bodies()
     */
    virtual void calcForce(bool calcVariances) = 0;
};


/** List of pointers to Force */
typedef std::vector<Force*> ForceList;


} // namespace StepCore


#endif
