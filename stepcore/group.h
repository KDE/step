/* This file is part of StepCore library.
   Copyright (C) 2008 Aliona Kuznetsova <aliona.kuz@gmail.com>
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
#ifndef STEPCORE_GROUP_H
#define STEPCORE_GROUP_H

#include "world.h"
#include "vector.h"
#include "object.h"

namespace StepCore {

class Group: public ItemGroup
{
    STEPCORE_OBJECT(Group)

public:
    explicit Group(QString name = QString());//Vector2d position = Vector2d(0),
            //Vector2d velocity = Vector2d(0));//XXX

    /** Get position of the group */
    Vector2d position() const;
    /** Set position of the group */
    void setPosition(const Vector2d& position);
    
    /** Get velocity of the particle */
    Vector2d velocity() const;
    /** Set velocity of the particle */
    void setVelocity(const Vector2d& velocity); 
    
    /** Get size of the group */
    Vector2d size() const;
    /** Set size of the group */
    void setSize(const Vector2d& size);

protected:

};

} // namespace StepCore

#endif

