/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
    SPDX-FileCopyrightText: 2014 Inge Wallin <inge@lysator.liu.se>

    SPDX-License-Identifier: GPL-2.0-or-later
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
