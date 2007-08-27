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

#include "tool.h"
#include "particle.h"
#include "rigidbody.h"

namespace StepCore {

STEPCORE_META_OBJECT(Note, "Note: displays a textual note on the scene", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Tool),
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, "m", "Position", position, setPosition)
    STEPCORE_PROPERTY_RW(QString, text, STEPCORE_UNITS_NULL, "Text", text, setText)
    )

STEPCORE_META_OBJECT(Graph, "Graph: plots a graph of any properties", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Tool),
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, "m", "Graph position on the scene", position, setPosition)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, size, "m", "Graph size on the scene", size, setSize)
    STEPCORE_PROPERTY_RW(QString, objectX, STEPCORE_UNITS_NULL, "X axis: object", objectX, setObjectX)
    STEPCORE_PROPERTY_RW(QString, propertyX, STEPCORE_UNITS_NULL, "X axis: object property", propertyX, setPropertyX)
    STEPCORE_PROPERTY_RW(int, indexX, STEPCORE_UNITS_NULL, "X axis: vector index", indexX, setIndexX)
    STEPCORE_PROPERTY_RW(QString, objectY, STEPCORE_UNITS_NULL, "Y axis: object", objectY, setObjectY)
    STEPCORE_PROPERTY_RW(QString, propertyY, STEPCORE_UNITS_NULL, "Y axis: property", propertyY, setPropertyY)
    STEPCORE_PROPERTY_RW(int, indexY, STEPCORE_UNITS_NULL, "Y axis: vector index", indexY, setIndexY)
    STEPCORE_PROPERTY_RW(bool, autoLimitsX, STEPCORE_UNITS_NULL, "Auto-limits along X axis", autoLimitsX, setAutoLimitsX)
    STEPCORE_PROPERTY_RW(bool, autoLimitsY, STEPCORE_UNITS_NULL, "Auto-limits along Y axis", autoLimitsY, setAutoLimitsY)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, limitsX, STEPCORE_UNITS_NULL, "Limits along X axis", limitsX, setLimitsX)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, limitsY, STEPCORE_UNITS_NULL, "Limits along Y axis", limitsY, setLimitsY)
    STEPCORE_PROPERTY_RW(bool, showPoints, STEPCORE_UNITS_NULL, "Show points on the graph", showPoints, setShowPoints)
    STEPCORE_PROPERTY_RW(bool, showLines, STEPCORE_UNITS_NULL, "Show lines on the graph", showLines, setShowLines)
    STEPCORE_PROPERTY_R (StepCore::Vector2d, currentValue, STEPCORE_UNITS_NULL, "Current value", currentValue)
    STEPCORE_PROPERTY_RW_D(std::vector<StepCore::Vector2d>, points, STEPCORE_UNITS_NULL, "points", points, setPoints)
    STEPCORE_PROPERTY_R (QString, unitsX, STEPCORE_UNITS_NULL, "Units along X axis", unitsX)
    STEPCORE_PROPERTY_R (QString, unitsY, STEPCORE_UNITS_NULL, "Units along Y axis", unitsY)
    )

STEPCORE_META_OBJECT(Meter, "Meter: displays any property on the scene", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Tool),
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, "m", "Meter position on the scene", position, setPosition)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, size, "m", "Meter size on the scene", size, setSize)
    STEPCORE_PROPERTY_RW(QString, object, STEPCORE_UNITS_NULL, "Observed object", object, setObject)
    STEPCORE_PROPERTY_RW(QString, property, STEPCORE_UNITS_NULL, "Observed property", property, setProperty)
    STEPCORE_PROPERTY_RW(int, index, STEPCORE_UNITS_NULL, "Vector index", index, setIndex)
    STEPCORE_PROPERTY_RW(int, digits, STEPCORE_UNITS_NULL, "Display digits", digits, setDigits)
    STEPCORE_PROPERTY_R (double, value, STEPCORE_UNITS_NULL, "Value", value)
    STEPCORE_PROPERTY_R (QString, units, STEPCORE_UNITS_NULL, "Units of measured property", units)
    )

STEPCORE_META_OBJECT(Controller, "Controller: allows to easily control any property", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Tool),
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, "m", "Controller position on the scene", position, setPosition)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, size, "m", "Controller size on the scene", size, setSize)
    STEPCORE_PROPERTY_RW(QString, object, STEPCORE_UNITS_NULL, "Controlled object", object, setObject)
    STEPCORE_PROPERTY_RW(QString, property, STEPCORE_UNITS_NULL, "Controlled property property", property, setProperty)
    STEPCORE_PROPERTY_RW(int, index, STEPCORE_UNITS_NULL, "Vector index", index, setIndex)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, limits, STEPCORE_UNITS_NULL, "Limits", limits, setLimits)
    STEPCORE_PROPERTY_RW(QString, increaseShortcut, STEPCORE_UNITS_NULL,
                            "Shortcut to increase the value", increaseShortcut, setIncreaseShortcut)
    STEPCORE_PROPERTY_RW(QString, decreaseShortcut, STEPCORE_UNITS_NULL,
                            "Shortcut to decrease the value", decreaseShortcut, setDecreaseShortcut)
    STEPCORE_PROPERTY_RW(double, increment, STEPCORE_UNITS_NULL, "Increment value", increment, setIncrement)
    STEPCORE_PROPERTY_RWF(double, value, STEPCORE_UNITS_NULL, "Value",
                            MetaProperty::DYNAMIC | MetaProperty::SIDEEFFECTS, value, setValue)
    STEPCORE_PROPERTY_R (QString, units, STEPCORE_UNITS_NULL, "Units of controlled property", units)
    )

STEPCORE_META_OBJECT(Tracer, "Tracer: traces trajectory of a point on a body", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Tool),
    STEPCORE_PROPERTY_RW(QString, body, STEPCORE_UNITS_NULL, "Traced body", body, setBody)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, localPosition, "m",
                    "Local position", localPosition, setLocalPosition)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, position, "m", "Position", position)
    STEPCORE_PROPERTY_RW_D(std::vector<StepCore::Vector2d>, points, "m", "points", points, setPoints)
    )

namespace {

static double variantToDouble(const QVariant& v, int index, bool *ok)
{
    if(ok) *ok = true;

    if(v.userType() == qMetaTypeId<Vector2d>()) {
        if(index >= 0 && index < 2) return v.value<Vector2d>()[index];
    } else {
        if(index == -1) return v.toDouble(ok);
    }

    if(ok) *ok = false;
    return 0.0;
}

static QVariant doubleToVariant(const QVariant& v, int index, double newV, bool *ok)
{
    if(ok) *ok = true;

    if(v.userType() == qMetaTypeId<Vector2d>()) {
        if(index >= 0 && index < 2) {
            Vector2d vector = v.value<Vector2d>();
            vector[index] = newV;
            return QVariant::fromValue(vector);
        }
    } else {
        if(index == -1) {
            bool ok1; v.toDouble(&ok1);
            if(ok1) return newV;
        }
    }

    if(ok) *ok = false;
    return 0.0;
}

}

Note::Note(Vector2d position, QString text)
    : _position(position), _text(text)
{
}

Graph::Graph(Vector2d position, Vector2d size)
    : _position(position), _size(size),
      _objectXPtr(0), _propertyX(), _indexX(-1),
      _objectYPtr(0), _propertyY(), _indexY(-1),
      _autoLimitsX(true), _autoLimitsY(true),
      _limitsX(0,1), _limitsY(0,1),
      _showLines(true), _showPoints(false)
{
}

bool Graph::isValidX() const
{
    bool ok;
    const MetaProperty* prX = propertyXPtr(); if(!prX) return false;
    variantToDouble(prX->readVariant(_objectXPtr), _indexX, &ok);
    return ok;
}

bool Graph::isValidY() const
{
    bool ok;
    const MetaProperty* prY = propertyYPtr(); if(!prY) return false;
    variantToDouble(prY->readVariant(_objectYPtr), _indexY, &ok);
    return ok;
}

Vector2d Graph::currentValue(bool* ok) const
{
    const MetaProperty* prX = propertyXPtr();
    const MetaProperty* prY = propertyYPtr();

    if(prX && prY) {
        bool ok1, ok2;
        Vector2d point(variantToDouble(prX->readVariant(_objectXPtr), _indexX, &ok1),
                       variantToDouble(prY->readVariant(_objectYPtr), _indexY, &ok2));
        if(ok1 && ok2) {
            if(ok) *ok = true;
            return point;
        }
    }

    if(ok) *ok = false;
    return Vector2d(0);
}

Vector2d Graph::recordPoint(bool* ok)
{
    bool ok1;
    Vector2d point(currentValue(&ok1));
    if(ok1) {
        if(ok) *ok = true;
        _points.push_back(point);
        return point;
    }

    if(ok) *ok = false;
    return Vector2d(0);
}

void Graph::worldItemRemoved(Item* item)
{
    if(item == 0) return;
    if(item == _objectXPtr) setObjectXPtr(0);
    if(item == _objectYPtr) setObjectYPtr(0);
}

void Graph::setWorld(World* world)
{
    if(world == NULL) {
        setObjectXPtr(0);
        setObjectYPtr(0);
    } else if(this->world() != NULL) { 
        if(_objectXPtr != NULL) _objectXPtr = world->object(_objectXPtr->name());
        if(_objectYPtr != NULL) _objectYPtr = world->object(_objectYPtr->name());
    }
    Item::setWorld(world);
}

QString Graph::unitsX() const
{
    const MetaProperty* pr = propertyXPtr();
    if(!pr && !isValidX()) return QString();

    return pr->units();
}

QString Graph::unitsY() const
{
    const MetaProperty* pr = propertyYPtr();
    if(!pr && !isValidY()) return QString();

    return pr->units();
}

Meter::Meter(Vector2d position, Vector2d size)
    : _position(position), _size(size),
      _objectPtr(0), _property(), _index(-1),
      _digits(7)
{
}

bool Meter::isValid() const
{
    bool ok;
    const MetaProperty* pr = propertyPtr(); if(!pr) return false;
    variantToDouble(pr->readVariant(_objectPtr), _index, &ok);
    return ok;
}

double Meter::value(bool* ok) const
{
    const MetaProperty* pr = propertyPtr();

    if(pr) {
        bool ok1;
        double v = variantToDouble(pr->readVariant(_objectPtr), _index, &ok1);
        if(ok1) {
            if(ok) *ok = true;
            return v;
        }
    }

    if(ok) *ok = false;
    return 0;
}

QString Meter::units() const
{
    const MetaProperty* pr = propertyPtr();
    if(!pr && !isValid()) return QString();

    return pr->units();
}

void Meter::worldItemRemoved(Item* item)
{
    if(item == 0) return;
    if(item == _objectPtr) setObjectPtr(0);
}

void Meter::setWorld(World* world)
{
    if(world == NULL) {
        setObjectPtr(0);
    } else if(this->world() != NULL) { 
        if(_objectPtr != NULL) _objectPtr = world->object(_objectPtr->name());
    }
    Item::setWorld(world);
}

Controller::Controller(Vector2d position, Vector2d size)
    : _position(position), _size(size),
      _objectPtr(0), _property(), _index(-1),
      _limits(-1,1), _increment(0.1)
{
}

bool Controller::isValid() const
{
    bool ok;
    const MetaProperty* pr = propertyPtr(); if(!pr) return false;
    variantToDouble(pr->readVariant(_objectPtr), _index, &ok);
    return ok && pr->isWritable();
}

double Controller::value(bool* ok) const
{
    const MetaProperty* pr = propertyPtr();

    if(pr && pr->isWritable()) {
        bool ok1;
        double v = variantToDouble(pr->readVariant(_objectPtr), _index, &ok1);
        if(ok1) {
            if(ok) *ok = true;
            return v;
        }
    }

    if(ok) *ok = false;
    return 0;
}

void Controller::setValue(double value, bool* ok = 0)
{
    const MetaProperty* pr = propertyPtr();

    if(pr && pr->isWritable()) {
        bool ok1;
        QVariant v = doubleToVariant(pr->readVariant(_objectPtr), _index, value, &ok1);
        if(ok1) {
            if(ok) *ok = true;
            pr->writeVariant(_objectPtr, v);
        }
    }

    if(ok) *ok = false;
}

QString Controller::units() const
{
    const MetaProperty* pr = propertyPtr();
    if(!pr && !isValid()) return QString();

    return pr->units();
}

void Controller::worldItemRemoved(Item* item)
{
    if(item == 0) return;
    if(item == _objectPtr) setObjectPtr(0);
}

void Controller::setWorld(World* world)
{
    if(world == NULL) {
        setObjectPtr(0);
    } else if(this->world() != NULL) { 
        if(_objectPtr != NULL) _objectPtr = world->object(_objectPtr->name());
    }
    Item::setWorld(world);
}

Tracer::Tracer(Body* bodyPtr, const Vector2d& localPosition)
    : _localPosition(localPosition)
{
    setColor(0xff0000ff);
    setBodyPtr(bodyPtr);
}

void Tracer::setBodyPtr(Body* bodyPtr)
{
    if(dynamic_cast<Particle*>(bodyPtr) || dynamic_cast<RigidBody*>(bodyPtr)) {
        //_localPosition1.setZero();
        _bodyPtr = bodyPtr;
    } else {
        //Particle* p1 = dynamic_cast<Particle*>(_bodyPtr1);
        //RigidBody* r1 = dynamic_cast<RigidBody*>(_bodyPtr1);
        //if(p1) _localPosition1 = p1->position();
        //else if(r1) _localPosition1 = r1->position();
        _bodyPtr = NULL;
    }
}

Vector2d Tracer::position() const
{
    Particle* p1 = dynamic_cast<Particle*>(_bodyPtr);
    if(p1) return p1->position() + _localPosition;

    RigidBody* r1 = dynamic_cast<RigidBody*>(_bodyPtr);
    if(r1) return r1->pointLocalToWorld(_localPosition);

    return _localPosition;
}

void Tracer::worldItemRemoved(Item* item)
{
    if(item == NULL) return;
    if(item == dynamic_cast<Item*>(_bodyPtr)) {
        _bodyPtr = NULL;
    }
}

void Tracer::setWorld(World* world)
{
    if(world == NULL) {
        _bodyPtr = NULL;
    } else if(this->world() != NULL) { 
        if(_bodyPtr != NULL)
            _bodyPtr = dynamic_cast<Body*>(world->item(body()));
    }
    Item::setWorld(world);
}

} // namespace StepCore

