/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
    SPDX-FileCopyrightText: 2014 Inge Wallin <inge@lysator.liu.se>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit ObjectErrors(Item* owner = nullptr): _owner(owner) {}

    /** Get the owner of ObjectErrors */
    Item* owner() const { return _owner; }
    /** Set the owner of ObjectErrors */
    void setOwner(Item* owner) { _owner = owner; }

private:
    Item* _owner;
};



} // namespace StepCore


#endif
