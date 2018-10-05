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

/** \file objecterrors.h
 *  \brief Contains the Objecterrors object.
 */

#ifndef STEPCORE_OBJECTERRORS_H
#define STEPCORE_OBJECTERRORS_H


#include <vector> // XXX: Replace if Qt is enabled.

#include "types.h"
#include "object.h"


namespace StepCore
{


class Item;


/** \ingroup errors
 *  \brief Base class for all errors objects
 */
class ObjectErrors: public Object
{
    STEPCORE_OBJECT(ObjectErrors)

public:
    /** Constructs ObjectErrors */
    explicit ObjectErrors(Item* owner = NULL): _owner(owner) {}

    /** Get the owner of ObjectErrors */
    Item* owner() const { return _owner; }
    /** Set the owner of ObjectErrors */
    void setOwner(Item* owner) { _owner = owner; }

private:
    Item* _owner;
};



} // namespace StepCore


#endif
