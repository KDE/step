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

/** \ingroup tools
 *  \brief Textual note item
 *
 *  Actual displaying of the Note should be
 *  implemented by application
 */
class Note: public Item, public Tool
{
    STEPCORE_OBJECT(Note)

public:
    /** Constructs Note */
    explicit Note(Vector2d position = Vector2d(0), QString text = QString());

    /** Get position of the note */
    const Vector2d& position() const { return _position; }
    /** Set position of the note */
    void setPosition(const Vector2d& position) { _position = position; }

    /** Get note text */
    const QString& text() const { return _text; }
    /** Set note text */
    void setText(const QString& text) { _text = text; }

protected:
    Vector2d _position;
    QString  _text;
};

/** \ingroup tools
 *  \brief Graph item
 *
 *  Actual updating and displaying of the Graph
 *  should be implemented by application
 */
class Graph: public Item, public Tool
{
    STEPCORE_OBJECT(Graph)

public:
    /** Constructs Graph */
    explicit Graph(Vector2d position = Vector2d(0), Vector2d size = Vector2d(400,300));

    /** Get position of the graph */
    const Vector2d& position() const { return _position; }
    /** Set position of the graph */
    void setPosition(const Vector2d& position) { _position = position; }

    /** Get size of the graph */
    const Vector2d& size() const { return _size; }
    /** Set size of the graph */
    void setSize(const Vector2d& size) { _size = size; }

    /** Get pointer to the objects for X axis */
    const Object* objectXPtr() const { return _objectXPtr; }
    /** Set pointer to the objects for X axis */
    void setObjectXPtr(const Object* objectXPtr) { _objectXPtr = objectXPtr; }

    /** Get name of the objects for X axis */
    QString objectX() const { return _objectXPtr ? _objectXPtr->name() : QString(); }
    /** Set name of the objects for X axis */
    void setObjectX(const QString& objectX) { setObjectXPtr(world()->object(objectX)); }
    
    /** Get name of the property for X axis */
    QString propertyX() const { return _propertyX; }
    /** Set name of the property for X axis */
    void setPropertyX(const QString& propertyX) { _propertyX = propertyX; }

    /** Get vector index for the X axis */
    int indexX() const { return _indexX; }
    /** Set vector index for the X axis */
    void setIndexX(int indexX) { _indexX = indexX; }

    /** Get pointer to the objects for Y axis */
    const Object* objectYPtr() const { return _objectYPtr; }
    /** Set pointer to the objects for Y axis */
    void setObjectYPtr(const Object* objectYPtr) { _objectYPtr = objectYPtr; }

    /** Get name of the objects for Y axis */
    QString objectY() const { return _objectYPtr ? _objectYPtr->name() : QString(); }
    /** Set name of the objects for Y axis */
    void setObjectY(const QString& objectY) { setObjectYPtr(world()->object(objectY)); }

    /** Get name of the property for Y axis */
    QString propertyY() const { return _propertyY; }
    /** Set name of the property for Y axis */
    void setPropertyY(const QString& propertyY) { _propertyY = propertyY; }

    /** Get vector index for the Y axis */
    int indexY() const { return _indexY; }
    /** Set vector index for the Y axis */
    void setIndexY(int indexY) { _indexY = indexY; }

    /** Get auto-limits for X axis */
    bool autoLimitsX() const { return _autoLimitsX; }
    /** Set auto-limits for X axis */
    void setAutoLimitsX(bool autoLimitsX) { _autoLimitsX = autoLimitsX; }

    /** Get auto-limits for Y axis */
    bool autoLimitsY() const { return _autoLimitsY; }
    /** Set auto-limits for Y axis */
    void setAutoLimitsY(bool autoLimitsY) { _autoLimitsY = autoLimitsY; }

    /** Get limits for X axis */
    const Vector2d& limitsX() const { return _limitsX; }
    /** Set limits for X axis */
    void setLimitsX(const Vector2d& limitsX) { _limitsX = limitsX; }

    /** Get limits for Y axis */
    const Vector2d& limitsY() const { return _limitsY; }
    /** Set limits for Y axis */
    void setLimitsY(const Vector2d& limitsY) { _limitsY = limitsY; }

    /** Get show-lines flag */
    bool showLines() const { return _showLines; }
    /** Set show-lines flag */
    void setShowLines(bool showLines) { _showLines = showLines; }

    /** Get show-points flag */
    bool showPoints() const { return _showPoints; }
    /** Set show-points flag */
    void setShowPoints(bool showPoints) { _showPoints = showPoints; }

    /** Get points list */
    const std::vector<Vector2d>& points() const { return _points; }
    /** Set points list */
    void setPoints(const std::vector<Vector2d>& points) { _points = points; }

    /** Get pointer to the property for X axis (or zero if not defined) */
    const MetaProperty* propertyXPtr() const {
        return _objectXPtr ? _objectXPtr->metaObject()->property(_propertyX) : 0;
    }

    /** Get pointer to the property for Y axis (or zero if not defined) */
    const MetaProperty* propertyYPtr() const {
        return _objectYPtr ? _objectYPtr->metaObject()->property(_propertyY) : 0;
    }

    /** Returns true if X-axis data source is valid */
    bool isValidX() const;
    /** Returns true if Y-axis data source is valid */
    bool isValidY() const;
    /** Returns true if X- and Y-axis data source is valid */
    bool isValid() const { return isValidX() && isValidY(); }

    /** Get current point value */
    Vector2d currentValue() const { return currentValue(0); }

    /** Get current point value
     *  \param ok Will indicate success of operation (if not null) */
    Vector2d currentValue(bool* ok) const;

    /** Get current point value and add it to points list
     *  \param ok Will indicate success of operation (if not null) */
    Vector2d recordPoint(bool* ok = 0);

    /** Clear points list */
    void clearPoints();

    void worldItemRemoved(Item* item);
    void setWorld(World* world);

protected:
    Vector2d _position;
    Vector2d _size;

    const Object* _objectXPtr;
    QString       _propertyX;
    int           _indexX;

    const Object* _objectYPtr;
    QString       _propertyY;
    int           _indexY;

    bool        _autoLimitsX;
    bool        _autoLimitsY;

    Vector2d    _limitsX;
    Vector2d    _limitsY;

    bool _showLines;
    bool _showPoints;

    std::vector<Vector2d> _points;
};

/** \ingroup tools
 *  \brief Controller item to control properties of other objects
 *
 *  Actual displaying of the Controller and its user interaction
 *  should be implemented by application
 */
class Controller: public Item, public Tool
{
    STEPCORE_OBJECT(Controller)

public:
    /** Constructs Controller */
    explicit Controller(Vector2d position = Vector2d(0), Vector2d size = Vector2d(200,60));

    /** Get position of the Controller */
    const Vector2d& position() const { return _position; }
    /** Set position of the Controller */
    void setPosition(const Vector2d& position) { _position = position; }

    /** Get size of the Controller */
    const Vector2d& size() const { return _size; }
    /** Set size of the Controller */
    void setSize(const Vector2d& size) { _size = size; }

    /** Get pointer to the controlled object */
    const Object* objectPtr() const { return _objectPtr; }
    /** Set pointer to the controlled object */
    void setObjectPtr(Object* objectPtr) { _objectPtr = objectPtr; }

    /** Get name of the controlled object */
    QString object() const { return _objectPtr ? _objectPtr->name() : QString(); }
    /** Set name of the controlled object */
    void setObject(const QString& object) { setObjectPtr(world()->object(object)); }
    
    /** Get name of the controlled property */
    QString property() const { return _property; }
    /** Set name of the controlled property */
    void setProperty(const QString& property) { _property = property; }

    /** Get vector index of the controlled property */
    int index() const { return _index; }
    /** Set vector index of the controlled property */
    void setIndex(int index) { _index = index; }

    /** Get GUI slider limits */
    const Vector2d& limits() const { return _limits; }
    /** Set GUI slider limits */
    void setLimits(const Vector2d& limits) { _limits = limits; }

    /** Get GUI increase shortcut */
    const QString& decreaseShortcut() const { return _decreaseShortcut; }
    /** Set GUI increase shortcut */
    void setDecreaseShortcut(const QString& decreaseShortcut) { _decreaseShortcut = decreaseShortcut; }

    /** Get GUI decrease shortcut */
    const QString& increaseShortcut() const { return _increaseShortcut; }
    /** Set GUI decrease shortcut */
    void setIncreaseShortcut(const QString& increaseShortcut) { _increaseShortcut = increaseShortcut; }

    /** Get increment step */
    double increment() const { return _increment; }
    /** Set increment step */
    void setIncrement(double increment) { _increment = increment; }

    /** Returns true if controlled property is valid */
    bool isValid() const;

    /** Get pointer to the controlled property */
    const MetaProperty* propertyPtr() const {
        return _objectPtr ? _objectPtr->metaObject()->property(_property) : 0;
    }

    /** Get value of the controlled property */
    double value() const { return value(0); }
    /** Set value of the controlled property */
    void setValue(double value) { setValue(value, 0); }

    /** Get value of the controlled property
     *  \param ok Will indicate success of operation (if not null) */
    double value(bool* ok) const;
    /** Set value of the controlled property
     *  \param ok Will indicate success of operation (if not null) */
    void setValue(double value, bool* ok);

    void worldItemRemoved(Item* item);
    void setWorld(World* world);

protected:
    Vector2d _position;
    Vector2d _size;

    Object*  _objectPtr;
    QString  _property;
    int      _index;

    Vector2d _limits;
    QString  _decreaseShortcut;
    QString  _increaseShortcut;

    double   _increment;
};

} // namespace StepCore

#endif
