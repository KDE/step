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

// TODO: split this file

#ifndef STEPCORE_TOOL_H
#define STEPCORE_TOOL_H

#include "world.h"
#include "types.h"

#include <QByteArray>
#include <QHash>
#include <QStringList>

namespace StepCore
{

/** \ingroup tools
 *  \brief Image embedded in Note
 */
class NoteImage: public Item
{
    STEPCORE_OBJECT(NoteImage)

public:
    /** Constructs NoteImage */
    explicit NoteImage(const QString& name = QString(),
                       const QByteArray& image = QByteArray())
                            : Item(name), _image(image) {}

    /** Get image data */
    const QByteArray& image() const { return _image; }

    /** Set image data */
    void setImage(const QByteArray& image) { _image = image; }

protected:
    QByteArray _image;
};

/** \ingroup tools
 *  \brief LaTeX formula embedded in Note
 */
class NoteFormula: public NoteImage
{
    STEPCORE_OBJECT(NoteFormula)

public:
    /** Constructs NoteFormula */
    explicit NoteFormula(const QString& name = QString(),
                         const QByteArray& image = QByteArray(),
                         const QString& code = QString())
                            : NoteImage(name, image), _code(code) {}

    /** Get formula code */
    const QString& code() const { return _code; }
    /** Set formula code */
    void setCode(const QString& code) { _code = code; }

protected:
    QString    _code;
};

/** \ingroup tools
 *  \brief Textual note item
 *
 *  Actual displaying of the Note should be
 *  implemented by application
 */
class Note: public ItemGroup, public Tool
{
    STEPCORE_OBJECT(Note)

public:
    /** Constructs Note */
    explicit Note(const Vector2d &position = Vector2d::Zero(),
            const Vector2d &size = Vector2d(250,100), const QString &text = QString());

    /** Get position of the note */
    const Vector2d& position() const { return _position; }
    /** Set position of the note */
    void setPosition(const Vector2d& position) { _position = position; }

    /** Get size of the note */
    const Vector2d& size() const { return _size; }
    /** Set size of the note */
    void setSize(const Vector2d& size) { _size = size.array().abs(); }

    /** Get note text */
    const QString& text() const { return _text; }
    /** Set note text */
    void setText(const QString& text) { _text = text; }

protected:
    Vector2d _position;
    Vector2d _size;
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
    explicit Graph(const Vector2d &position = Vector2d::Zero(), const Vector2d &size = Vector2d(400,300));

    /** Get position of the graph */
    const Vector2d& position() const { return _position; }
    /** Set position of the graph */
    void setPosition(const Vector2d& position) { _position = position; }

    /** Get size of the graph */
    const Vector2d& size() const { return _size; }
    /** Set size of the graph */
    void setSize(const Vector2d& size) { _size = size.array().abs(); }

    /** Get pointer to the objects for X axis */
    Object* objectX() const { return _objectX; }
    /** Set pointer to the objects for X axis */
    void setObjectX(Object* objectX) { _objectX = objectX; }

    /** Get name of the property for X axis */
    QString propertyX() const { return _propertyX; }
    /** Set name of the property for X axis */
    void setPropertyX(const QString& propertyX) { _propertyX = propertyX; }

    /** Get vector index for the X axis */
    int indexX() const { return _indexX; }
    /** Set vector index for the X axis */
    void setIndexX(int indexX) { _indexX = indexX; }

    /** Get pointer to the objects for Y axis */
    Object* objectY() const { return _objectY; }
    /** Set pointer to the objects for Y axis */
    void setObjectY(Object* objectY) { _objectY = objectY; }

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
    const Vector2dList& points() const { return _points; }
    /** Set points list */
    void setPoints(const Vector2dList& points) { _points = points; }

    /** Get pointer to the property for X axis (or zero if not defined) */
    const MetaProperty* propertyXPtr() const {
        return _objectX ? _objectX->metaObject()->property(_propertyX) : 0;
    }

    /** Get pointer to the property for Y axis (or zero if not defined) */
    const MetaProperty* propertyYPtr() const {
        return _objectY ? _objectY->metaObject()->property(_propertyY) : 0;
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
    void clearPoints() { _points.clear(); }

    /** Return units of propertyX */
    QString unitsX() const;

    /** Return units of propertyY */
    QString unitsY() const;

    //void worldItemRemoved(Item* item);
    //void setWorld(World* world);

protected:
    Vector2d _position;
    Vector2d _size;

    Object* _objectX;
    QString       _propertyX;
    int           _indexX;

    Object* _objectY;
    QString       _propertyY;
    int           _indexY;

    bool        _autoLimitsX;
    bool        _autoLimitsY;

    Vector2d    _limitsX;
    Vector2d    _limitsY;

    bool _showLines;
    bool _showPoints;

    Vector2dList _points;
};

/** \ingroup tools
 *  \brief Meter to observe properties of other objects
 *
 *  Actual displaying of the Meter and its user interaction
 *  should be implemented by application
 */
class Meter: public Item, public Tool
{
    STEPCORE_OBJECT(Meter)

public:
    /** Constructs Meter */
    explicit Meter(const Vector2d &position = Vector2d::Zero(), const Vector2d &size = Vector2d(70,24));

    /** Get position of the meter */
    const Vector2d& position() const { return _position; }
    /** Set position of the meter */
    void setPosition(const Vector2d& position) { _position = position; }

    /** Get size of the meter */
    const Vector2d& size() const { return _size; }
    /** Set size of the meter */
    void setSize(const Vector2d& size) { _size = size.array().abs(); }

    /** Get pointer to the observed object */
    Object* object() const { return _object; }
    /** Set pointer to the observed object */
    void setObject(Object* object) { _object = object; }

    /** Get name of the observed property */
    QString property() const { return _property; }
    /** Set name of the observed property */
    void setProperty(const QString& property) { _property = property; }

    /** Get vector index of the observed property */
    int index() const { return _index; }
    /** Set vector index of the observed property */
    void setIndex(int index) { _index = index; }

    /** Get display digits */
    int digits() const { return _digits; }
    /** Set display digits */
    void setDigits(int digits) { _digits = digits; }

    /** Returns true if observed property is valid */
    bool isValid() const;

    /** Get pointer to the observed property */
    const MetaProperty* propertyPtr() const {
        return _object ? _object->metaObject()->property(_property) : 0;
    }

    /** Get value of the observed property */
    double value() const { return value(0); }

    /** Set value of the controlled property */
    virtual void setValue(double) {}

    /** Get value of the observed property
     *  \param ok Will indicate success of operation (if not null) */
    double value(bool* ok) const;

    /** Return units of measured property */
    QString units() const;

    //void worldItemRemoved(Item* item);
    //void setWorld(World* world);

protected:
    Vector2d _position;
    Vector2d _size;

    Object*  _object;
    QString  _property;
    int      _index;

    int      _digits;
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
    explicit Controller(const Vector2d &position = Vector2d::Zero(), const Vector2d &size = Vector2d(200,60));

    /** Get position of the Controller */
    const Vector2d& position() const { return _position; }
    /** Set position of the Controller */
    void setPosition(const Vector2d& position) { _position = position; }

    /** Get size of the Controller */
    const Vector2d& size() const { return _size; }
    /** Set size of the Controller */
    void setSize(const Vector2d& size) { _size = size.array().abs(); }

    /** Get pointer to the controlled object */
    Object* object() const { return _object; }
    /** Set pointer to the controlled object */
    void setObject(Object* object) { _object = object; }

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
        return _object ? _object->metaObject()->property(_property) : 0;
    }

    /** Get value of the controlled property */
    double value() const { return value(0); }
    /** Set value of the controlled property */
    void setValue(double value) { setValue(value, 0); }

    /** Get value of the controlled property
     *  \param ok Will indicate success of operation (if not null) */
    double value(bool* ok) const;
    /** Set value of the controlled property
     *  \param value New value for the property
     *  \param ok Will indicate success of operation (if not null) */
    void setValue(double value, bool* ok);

    /** Return units of measured property */
    QString units() const;

    //void worldItemRemoved(Item* item);
    //void setWorld(World* world);

protected:
    Vector2d _position;
    Vector2d _size;

    Object*  _object;
    QString  _property;
    int      _index;

    Vector2d _limits;
    QString  _decreaseShortcut;
    QString  _increaseShortcut;

    double   _increment;
};

class Particle;
class RigidBody;

/** \ingroup tools
 *  \brief Traces position of the body
 *
 *  Actual displaying of the Traces and its user interaction
 *  should be implemented by application
 */
class Tracer: public Item, public Tool
{
    STEPCORE_OBJECT(Tracer)

public:
    /** Constructs Spring */
    explicit Tracer(Object* body = 0, const Vector2d& localPosition = Vector2d::Zero());

    /** Get pointer to the first body */
    Object* body() const { return _body; }
    /** Set pointer to the first connected body */
    void setBody(Object* body);

    /** Local position of the tracer on the body
     *  or in the world (if the tracer is not connected) */
    Vector2d localPosition() const { return _localPosition; }
    /** Set local position of the tracer on the body
     *  or in the world (if the tracer is not connected) */
    void setLocalPosition(const Vector2d& localPosition) { _localPosition = localPosition; }

    /** Position of the tracer */
    Vector2d position() const;

    /** Get points list */
    const Vector2dList& points() const { return _points; }
    /** Set points list */
    void setPoints(const Vector2dList& points) { _points = points; }

    /** Get current position value and add it to points list */
    Vector2d recordPoint() { Vector2d p = position(); _points.push_back(p); return p; }

    /** Clear points list */
    void clearPoints() { _points.clear(); }

    //void worldItemRemoved(Item* item);
    //void setWorld(World* world);

protected:
    Object* _body;
    Vector2d _localPosition;
    Vector2dList _points;

    Particle*  _p;
    RigidBody* _r;
};

} // namespace StepCore

#endif
