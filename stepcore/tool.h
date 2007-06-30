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

/** \file tool.h
 *  \brief Note class
 */

#ifndef STEPCORE_TOOL_H
#define STEPCORE_TOOL_H

#include "world.h"
#include "types.h"

namespace StepCore
{

class Note: public Item, public Tool
{
    STEPCORE_OBJECT(Note)

public:
    Note(Vector2d position = Vector2d(0), QString text = QString());

    const QString& text() const { return _text; }
    void setText(const QString& text) { _text = text; }

    const Vector2d& position() const { return _position; }
    void setPosition(const Vector2d& position) { _position = position; }

private:
    Vector2d _position;
    QString  _text;
};

class Graph: public Item, public Tool
{
    STEPCORE_OBJECT(Graph)

public:
    Graph(Vector2d position = Vector2d(0), Vector2d size = Vector2d(400,400));

    const Vector2d& position() const { return _position; }
    void setPosition(const Vector2d& position) { _position = position; }

    const Vector2d& size() const { return _size; }
    void setSize(const Vector2d& size) { _size = size; }

    const Object* objectPtr1() const { return _objectPtr1; }
    void setObjectPtr1(const Object* objectPtr1) { _objectPtr1 = objectPtr1; setPropertyPtr1(0); }

    QString object1() const { return _objectPtr1 ? _objectPtr1->name() : QString(); }
    void setObject1(const QString& object1) { setObjectPtr1(world()->object(object1)); }

    const MetaProperty* propertyPtr1() const { return _propertyPtr1; }
    void setPropertyPtr1(const MetaProperty* propertyPtr1) { _propertyPtr1 = propertyPtr1; setIndex1(0); }
    
    QString property1() const { return _propertyPtr1 ? _propertyPtr1->name() : QString(); }
    void setProperty1(const QString& property1);

    int index1() const { return _index1; }
    void setIndex1(int index1) { _index1 = index1; }

    const Object* objectPtr2() const { return _objectPtr2; }
    void setObjectPtr2(const Object* objectPtr2) { _objectPtr2 = objectPtr2; setPropertyPtr2(0); }

    QString object2() const { return _objectPtr2 ? _objectPtr2->name() : QString(); }
    void setObject2(const QString& object2) { setObjectPtr2(world()->object(object2)); }

    const MetaProperty* propertyPtr2() const { return _propertyPtr2; }
    void setPropertyPtr2(const MetaProperty* propertyPtr2) { _propertyPtr2 = propertyPtr2; setIndex2(0); }

    QString property2() const { return _propertyPtr2 ? _propertyPtr2->name() : QString(); }
    void setProperty2(const QString& property2);

    int index2() const { return _index2; }
    void setIndex2(int index2) { _index2 = index2; }

    const Vector2d& limitsX() const { return _limitsX; }
    void setLimitsX(const Vector2d& limitsX) { _limitsX = limitsX; }

    const Vector2d& limitsY() const { return _limitsY; }
    void setLimitsY(const Vector2d& limitsY) { _limitsY = limitsY; }

    const std::vector<Vector2d>& points() const { return _points; }
    void setPoints(const std::vector<Vector2d>& points) { _points = points; }

    void clearPoints();
    Vector2d recordPoint(bool* ok = 0);
    Vector2d measurePoint(bool* ok = 0);

    void worldItemRemoved(Item* item);
    void setWorld(World* world);

private:
    double getValue(const QVariant& v, int index, bool* ok = 0);

private:
    Vector2d _position;
    Vector2d _size;

    const Object* _objectPtr1;
    const MetaProperty* _propertyPtr1;
    int _index1;

    const Object* _objectPtr2;
    const MetaProperty* _propertyPtr2;
    int _index2;

    Vector2d    _limitsX;
    Vector2d    _limitsY;

    std::vector<Vector2d> _points;
};

} // namespace StepCore

#endif