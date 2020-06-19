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

/** \file object.h
 *  \brief Object, MetaObject and MetaProperty classes
 */

#ifndef STEPCORE_OBJECT_H
#define STEPCORE_OBJECT_H

#include <QBitArray>
#include <QString>
#include <QVariant>

#include <Eigen/Core> // for EIGEN_MAKE_ALIGNED_OPERATOR_NEW

#include "util.h"

namespace StepCore {

class MetaObject;
class MetaProperty;

#define STEPCORE_OBJECT_NA(_className) \
    private: \
        typedef _className _thisType; \
        static const StepCore::MetaObject   _metaObject; \
        static const StepCore::MetaObject*  _superClasses[]; \
        static const StepCore::MetaProperty _classProperties[]; \
    public: \
        static  const StepCore::MetaObject* staticMetaObject() { return &_metaObject; } \
        virtual const StepCore::MetaObject* metaObject() const { return &_metaObject; } \
    private:

#define STEPCORE_OBJECT(_className) \
    public: \
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW \
    STEPCORE_OBJECT_NA(_className)

/** \ingroup reflections
 *  \brief Root of the StepCore classes hierarchy
 */
class Object
{
    STEPCORE_OBJECT(Object)

public:
    explicit Object(const QString& name = QString()): _name(name) {}
    virtual ~Object() {}

    /** Returns name of the object */
    const QString& name() const { return _name; }
    /** Set name of the object */
    void setName(const QString& name) { _name = name; }

protected:
    QString _name;
};

/** \ingroup reflections
 *  \brief Meta-information about property
 */
class MetaProperty
{
public:
    enum {
        READABLE = 1, ///< Property is readable
        WRITABLE = 2, ///< Property is writable
        STORED = 4,   ///< Property should be stored in file
        DYNAMIC = 32, ///< Variable changes during simulation or changes of other properties
        SIDEEFFECTS = 64 ///< Changing this property has side-effects
                         ///  (changes in other dynamic or non-dynamic properties)
                         ///  @note Do not set this together with STORED
    };

public:
    MetaProperty():
        _name(QLatin1String("")), _units(QString()), _description(QLatin1String("")),
        _flags(0), _userTypeId(0), _readVariant(0), _writeVariant(0),
        _readString(0), _writeString(0), _initialized(false) {}

    /*MetaProperty(const MetaProperty& p):
        _name(p._name), _units(p._units), _flags(p.flags), _userTypeId(p._userTypeId),
        _readVariant(p._readVariant), _writeVariant(p._writeVariant),
        _readString(p._readString), _writeString(p._writeString) {}*/
        
    MetaProperty(const QString& name, const QString& units,
                 const QString& description, int flags, int userType,
                 QVariant (*const readVariant)(const Object*),
                 bool (*const writeVariant)(Object* obj, const QVariant& v),
                 QString (*const readString)(const Object* obj),
                 bool (*const writeString)(Object* obj, const QString& v))
        : _name(name), _units(units), _description(description),
          _flags(flags), _userTypeId(userType),
          _readVariant(readVariant), _writeVariant(writeVariant),
          _readString(readString), _writeString(writeString), _initialized(false) {}

    /** Returns property name */
    const QString& name() const { return _name; }
    /** Returns property units (if appropriate) */
    const QString& units() const { return _units; }
    /** Returns property description */
    const QString& description() const { return _description; }
    /** Returns property flags */
    int flags() const { return _flags; }

    /** Returns translated property name */
    const QString& nameTr() const { tryInit(); return _nameTr; }
    /** Returns translated property units (if appropriate) */
    const QString& unitsTr() const { tryInit(); return _unitsTr; }
    /** Returns translated property description */
    const QString& descriptionTr() const { tryInit(); return _descriptionTr; }

    /** Returns property userType (see QMetaProperty) */
    int userTypeId() const { return _userTypeId; }
    /** Read property as QVariant */
    QVariant readVariant(const Object* obj) const { return _readVariant(obj); }
    /** Write property as QVariant. \return true on success */
    bool writeVariant(Object* obj, const QVariant& v) const { return _writeVariant(obj, v); }

    /** Read property as string */
    QString readString(const Object* obj) const { return _readString(obj); }
    /** Write property as string. \return true on success */
    bool writeString(Object* obj, const QString& s) const { return _writeString(obj, s); }

    /** Returns true if this property is readable */
    bool isReadable() const { return _flags & READABLE; }
    /** Returns true if this property is writable */
    bool isWritable() const { return _flags & WRITABLE; }
    /** Returns true if this property should be stored */
    bool isStored() const { return _flags & STORED; }
    /** Returns true if this property is dynamic (changes during simulation) */
    bool isDynamic() const { return _flags & DYNAMIC; }
    /** Returns true if this property has side-effects
     *  (changes in other dynamic or non-dynamic properties) */
    bool hasSideEffects() const { return _flags & SIDEEFFECTS; }

public:
    const QString _name;
    const QString _units;
    const QString _description;
    const int _flags;
    const int _userTypeId;
    QVariant (*const _readVariant)(const Object* obj);
    bool (*const _writeVariant)(Object* obj, const QVariant& v);
    QString (*const _readString)(const Object* obj);
    bool (*const _writeString)(Object* obj, const QString& v);

    mutable bool _initialized;
    mutable QString _nameTr;
    mutable QString _unitsTr;
    mutable QString _descriptionTr;

public:
    void init() const;
    void tryInit() const { if(!_initialized) init(); }
};

/** \ingroup reflections
 *  \brief Meta-information about class
 */
class MetaObject
{
public:
    enum {
        ABSTRACT = 1 ///< Class is abstract
    };

public:
    /** Returns class name */
    const QString& className() const { return _className; }
    /** Returns class description */
    const QString& description() const { return _description; }

    /** Returns translated class name */
    const QString& classNameTr() const { tryInit(); return _classNameTr; }
    /** Returns translated class description */
    const QString& descriptionTr() const { tryInit(); return _descriptionTr; } 

    /** Returns class id */
    int classId() const { tryInit(); return _classId; }

    /** Returns true if class is abstract */
    bool isAbstract() const { return _flags & ABSTRACT; }
    /** Creates new object of this class */
    Object* newObject() const { return _newObject(); }
    /** Creates a copy of given object */
    Object* cloneObject(const Object& obj) const { return _cloneObject(obj); }

    /** Returns number of direct bases */
    int superClassCount() const { return _superClassCount; }
    /** Returns direct base */
    const MetaObject* superClass(int n) const { return _superClasses[n]; }
    /** Returns true if this class inherits class described by obj */
    bool inherits(const MetaObject* obj) const;
    /** Returns true if this class inherits class T */
    template<class T> bool inherits() const { return inherits(T::staticMetaObject()); }
    /** Returns true if this class inherits class T */
    template<class T> bool inherits(T*) const { return inherits(T::staticMetaObject()); }
    /** Returns true if this class inherits class named name
     *  \note Due to technical reason this is much slower then
     *        inherits(const MetaObject*) function */
    bool inherits(const char* name) const;

    /** Returns number of non-inherited properties */
    int classPropertyCount() const { return _classPropertyCount; }
    /** Returns non-inherited property */
    const MetaProperty* classProperty(int n) const { return &_classProperties[n]; }

    /** Returns property count */
    int propertyCount() const { tryInit(); return _allPropertyCount; }
    /** Returns property by index */
    const MetaProperty* property(int n) const { tryInit(); return _allProperties[n]; }
    /** Returns property by name */
    const MetaProperty* property(const QString& name) const;

public:
    void init() const;
    void tryInit() const { if(!_initialized) init(); }
    void copyProperties(const MetaProperty** dest) const;

    const QString     _className;
    const QString     _description;
    const int         _flags;
    Object* (*const _newObject)();
    Object* (*const _cloneObject)(const Object&);

    const MetaObject**  const _superClasses;
    const int                 _superClassCount;
    const MetaProperty* const _classProperties;
    const int                 _classPropertyCount;

    mutable bool                 _initialized;
    mutable const MetaProperty** _allProperties;
    mutable int                  _allPropertyCount;

    mutable int _classId;
    mutable QBitArray _allSuperClassIds;

    mutable QString     _classNameTr;
    mutable QString     _descriptionTr;

    static int  s_classIdCount;
};

/** Casts between pointers to Object */
template<class _Dst, class _Src> // XXX: implement it better
_Dst stepcore_cast(_Src src) {
    if(!src || !src->metaObject()->template inherits(_Dst())) return NULL;
    return static_cast<_Dst>(src);
}

/* Helper functions TODO: namespace of class ? */

template<typename T> inline QString typeToString(const T& v) {
    return QVariant::fromValue(v).toString();
}

template<typename T> inline T stringToType(const QString& s, bool* ok) {
    QVariant v(s); *ok = v.convert((QVariant::Type) qMetaTypeId<T>());
    return v.value<T>();
}

template<typename T> inline QVariant typeToVariant(const T& v) {
    return QVariant::fromValue(v);
}

template<typename T> inline T variantToType(const QVariant& v, bool* ok) {
    if(v.userType() == qMetaTypeId<T>()) { *ok = true; return v.value<T>(); }
    QVariant vc(v); *ok = vc.convert((QVariant::Type)qMetaTypeId<T>());
    return vc.value<T>();
}

template<class C, typename T>
struct MetaPropertyHelper {

    /* read */
    template<T (C::*_read)() const> static QVariant read(const Object* obj) {
        return typeToVariant<T>((dynamic_cast<const C*>(obj)->*_read)());
    }
    template<const T& (C::*_read)() const> static QVariant read(const Object* obj) {
        return typeToVariant<T>((dynamic_cast<const C*>(obj)->*_read)());
    }

    /* write */
    template<void (C::*_write)(T)> static bool write(Object* obj, const QVariant& v) {
        bool ok; T tv = variantToType<T>(v, &ok); if(!ok) return false;
        (dynamic_cast<C*>(obj)->*_write)(tv); return true;
    }
    template<void (C::*_write)(const T&)> static bool write(Object* obj, const QVariant& v) {
        bool ok; T tv = variantToType<T>(v, &ok); if(!ok) return false;
        (dynamic_cast<C*>(obj)->*_write)(tv); return true;
    }
    template<bool (C::*_write)(T)> static bool write(Object* obj, const QVariant& v) {
        bool ok; T tv = variantToType<T>(v, &ok); if(!ok) return false;
        return (dynamic_cast<C*>(obj)->*_write)(tv);
    }
    template<bool (C::*_write)(const T&)> static bool write(Object* obj, const QVariant& v) {
        bool ok; T tv = variantToType<T>(v, &ok); if(!ok) return false;
        return (dynamic_cast<C*>(obj)->*_write)(tv);
    }

    /* readString */
    template<T (C::*_read)() const> static QString readString(const Object* obj) {
        return typeToString<T>((dynamic_cast<const C*>(obj)->*_read)());
    }
    template<const T& (C::*_read)() const> static QString readString(const Object* obj) {
        return typeToString<T>((dynamic_cast<const C*>(obj)->*_read)());
    }

    /* writeString */
    template<void (C::*_write)(T)> static bool writeString(Object* obj, const QString& s) {
        bool ok; T tv = stringToType<T>(s, &ok); if(!ok) return false;
        (dynamic_cast<C*>(obj)->*_write)(tv); return true;
    }
    template<void (C::*_write)(const T&)> static bool writeString(Object* obj, const QString& s) {
        bool ok; T tv = stringToType<T>(s, &ok); if(!ok) return false;
        (dynamic_cast<C*>(obj)->*_write)(tv); return true;
    }
    template<bool (C::*_write)(T)> static bool writeString(Object* obj, const QString& s) {
        bool ok; T tv = stringToType<T>(s, &ok); if(!ok) return false;
        return (dynamic_cast<C*>(obj)->*_write)(tv);
    }
    template<bool (C::*_write)(const T&)> static bool writeString(Object* obj, const QString& s) {
        bool ok; T tv = stringToType<T>(s, &ok); if(!ok) return false;
        return (dynamic_cast<C*>(obj)->*_write)(tv);
    }

    static QVariant readNull(const Object* obj) { Q_UNUSED(obj) return QVariant(); }
    static QString readStringNull(const Object* obj) { Q_UNUSED(obj) return QString(); }
    static bool writeNull(Object* obj, const QVariant& v) { Q_UNUSED(obj) Q_UNUSED(v) return false; }
    static bool writeStringNull(Object* obj, const QString& s) { Q_UNUSED(obj) Q_UNUSED(s) return false; }
};

template<typename Class, int N>
struct MetaObjectHelper {
    static Object* newObjectHelper() { return new Class(); }
    static Object* cloneObjectHelper(const Object& obj) {
        return new Class(static_cast<const Class&>(obj));
    }
};

template<class Class>
struct MetaObjectHelper<Class, MetaObject::ABSTRACT> {
    static Object* newObjectHelper() { return NULL; }
    static Object* cloneObjectHelper(const Object& obj) { Q_UNUSED(obj) return NULL; }
};

#define STEPCORE_FROM_UTF8(str) QString::fromUtf8(str)

#define STEPCORE_UNITS_NULL QString()
#define STEPCORE_UNITS_1 QString("")

#define _STEPCORE_PROPERTY_NULL StepCore::MetaProperty()

#define STEPCORE_META_OBJECT(_className, _classNameNoop, _description, _flags, __superClasses, __properties) \
    const StepCore::MetaProperty _className::_classProperties[] = { _STEPCORE_PROPERTY_NULL, __properties }; \
    const StepCore::MetaObject*  _className::_superClasses[] = { 0, __superClasses }; \
    const StepCore::MetaObject   _className::_metaObject = { \
        QString(STEPCORE_STRINGIFY(_className)), QString(_description), _flags, \
        StepCore::MetaObjectHelper<_className, _flags & StepCore::MetaObject::ABSTRACT>::newObjectHelper, \
        StepCore::MetaObjectHelper<_className, _flags & StepCore::MetaObject::ABSTRACT>::cloneObjectHelper, \
        _superClasses+1, sizeof(_superClasses)/sizeof(*_superClasses)-1, \
        _classProperties+1, sizeof(_classProperties)/sizeof(*_classProperties)-1, false, 0, 0, 0, QBitArray(), "", "" };
    
#define STEPCORE_SUPER_CLASS(_className) _className::staticMetaObject(),

#define STEPCORE_PROPERTY_RF(_type, _name, _nameNoop, _units, _description, _flags, _read) \
    StepCore::MetaProperty( STEPCORE_STRINGIFY(_name), _units, _description, \
      StepCore::MetaProperty::READABLE | _flags, qMetaTypeId<_type>(), \
      StepCore::MetaPropertyHelper<_thisType, _type>::read<&_thisType::_read>, \
      StepCore::MetaPropertyHelper<_thisType, _type>::writeNull, \
      StepCore::MetaPropertyHelper<_thisType, _type>::readString<&_thisType::_read>, \
      StepCore::MetaPropertyHelper<_thisType, _type>::writeStringNull ),

#define STEPCORE_PROPERTY_RWF(_type, _name, _nameNoop, _units, _description, _flags, _read, _write) \
    StepCore::MetaProperty( STEPCORE_STRINGIFY(_name), _units, _description, \
      StepCore::MetaProperty::READABLE | StepCore::MetaProperty::WRITABLE | _flags, \
      qMetaTypeId<_type>(), \
      StepCore::MetaPropertyHelper<_thisType, _type>::read<&_thisType::_read>, \
      StepCore::MetaPropertyHelper<_thisType, _type>::write<&_thisType::_write>, \
      StepCore::MetaPropertyHelper<_thisType, _type>::readString<&_thisType::_read>, \
      StepCore::MetaPropertyHelper<_thisType, _type>::writeString<&_thisType::_write> ),

#define STEPCORE_PROPERTY_R(_type, _name, _nameNoop, _units, _description, _read) \
    STEPCORE_PROPERTY_RF(_type, _name, _nameNoop, _units, _description, 0, _read)

#define STEPCORE_PROPERTY_RW(_type, _name, _nameNoop, _units, _description, _read, _write) \
    STEPCORE_PROPERTY_RWF(_type, _name, _nameNoop, _units, _description, \
        StepCore::MetaProperty::STORED, _read, _write)

#define STEPCORE_PROPERTY_R_D(_type, _name, _nameNoop, _units, _description, _read) \
    STEPCORE_PROPERTY_RF(_type, _name, _nameNoop, _units, _description, StepCore::MetaProperty::DYNAMIC, _read)

#define STEPCORE_PROPERTY_RW_D(_type, _name, _nameNoop, _units, _description, _read, _write) \
    STEPCORE_PROPERTY_RWF(_type, _name, _nameNoop, _units, _description, \
        StepCore::MetaProperty::STORED | StepCore::MetaProperty::DYNAMIC, _read, _write)

} // namespace StepCore

#endif

