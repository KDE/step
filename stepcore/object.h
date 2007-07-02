#ifndef STEPCORE_OBJECT_H
#define STEPCORE_OBJECT_H

/** \file object.h
 *  \brief Object, MetaObject and MetaProperty classes
 */

#include "util.h"

#include <QString>
#include <QVariant>

namespace StepCore {

class MetaObject;
class MetaProperty;

#define STEPCORE_OBJECT(_className) \
    private: \
        typedef _className _thisType; \
        static const StepCore::MetaObject   _metaObject; \
        static const StepCore::MetaObject*  _superClasses[]; \
        static const StepCore::MetaProperty _classProperties[]; \
    public: \
        static  const StepCore::MetaObject* staticMetaObject() { return &_metaObject; } \
        virtual const StepCore::MetaObject* metaObject() const { return &_metaObject; } \
    private:

/** \ingroup reflections
 *  \brief Root of the StepCore classes hierarchy
 */
class Object
{
    STEPCORE_OBJECT(Object)

public:
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
        //USER = 8      ///< Property should be shown in UI
        DYNAMIC = 32  ///< Variable changes during simulation
    };

public:
    /** Returns property name */
    const QString& name() const { return _name; }
    /** Returns property description */
    const QString& description() const { return _description; }
    /** Returns property flags */
    int flags() const { return _flags; }

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

public:
    const QString _name;
    const QString _description;
    const int _flags;
    const int _userTypeId;
    QVariant (*const _readVariant)(const Object* obj);
    bool (*const _writeVariant)(Object* obj, const QVariant& v);
    QString (*const _readString)(const Object* obj);
    bool (*const _writeString)(Object* obj, const QString& v);
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
    /** Returns true if this class inherits class named name */
    bool inherits(const char* name) const;

    /** Returns number of non-inherited properties */
    int classPropertyCount() const { return _classPropertyCount; }
    /** Returns non-inherited property */
    const MetaProperty* classProperty(int n) const { return &_classProperties[n]; }

    /** Returns property count */
    int propertyCount() const { if(!_initialized) init(); return _allPropertyCount; }
    /** Returns property by index */
    const MetaProperty* property(int n) const { if(!_initialized) init(); return _allProperties[n]; }
    /** Returns property by name */
    const MetaProperty* property(const QString& name) const;

public:
    void init() const;
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
};

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
        STEPCORE_ASSERT_NOABORT(dynamic_cast<const C*>(obj));
        return typeToVariant<T>((dynamic_cast<const C*>(obj)->*_read)());
    }
    template<const T& (C::*_read)() const> static QVariant read(const Object* obj) {
        STEPCORE_ASSERT_NOABORT(dynamic_cast<const C*>(obj));
        return typeToVariant<T>((dynamic_cast<const C*>(obj)->*_read)());
    }

    /* write */
    template<void (C::*_write)(T)> static bool write(Object* obj, const QVariant& v) {
        STEPCORE_ASSERT_NOABORT(dynamic_cast<const C*>(obj));
        bool ok; T tv = variantToType<T>(v, &ok); if(!ok) return false;
        (dynamic_cast<C*>(obj)->*_write)(tv); return true;
    }
    template<void (C::*_write)(const T&)> static bool write(Object* obj, const QVariant& v) {
        STEPCORE_ASSERT_NOABORT(dynamic_cast<const C*>(obj));
        bool ok; T tv = variantToType<T>(v, &ok); if(!ok) return false;
        (dynamic_cast<C*>(obj)->*_write)(tv); return true;
    }
    template<bool (C::*_write)(T)> static bool write(Object* obj, const QVariant& v) {
        STEPCORE_ASSERT_NOABORT(dynamic_cast<const C*>(obj));
        bool ok; T tv = variantToType<T>(v, &ok); if(!ok) return false;
        return (dynamic_cast<C*>(obj)->*_write)(tv);
    }
    template<bool (C::*_write)(const T&)> static bool write(Object* obj, const QVariant& v) {
        STEPCORE_ASSERT_NOABORT(dynamic_cast<const C*>(obj));
        bool ok; T tv = variantToType<T>(v, &ok); if(!ok) return false;
        return (dynamic_cast<C*>(obj)->*_write)(tv);
    }

    /* readString */
    template<T (C::*_read)() const> static QString readString(const Object* obj) {
        STEPCORE_ASSERT_NOABORT(dynamic_cast<const C*>(obj));
        return typeToString<T>((dynamic_cast<const C*>(obj)->*_read)());
    }
    template<const T& (C::*_read)() const> static QString readString(const Object* obj) {
        STEPCORE_ASSERT_NOABORT(dynamic_cast<const C*>(obj));
        return typeToString<T>((dynamic_cast<const C*>(obj)->*_read)());
    }

    /* writeString */
    template<void (C::*_write)(T)> static bool writeString(Object* obj, const QString& s) {
        STEPCORE_ASSERT_NOABORT(dynamic_cast<const C*>(obj));
        bool ok; T tv = stringToType<T>(s, &ok); if(!ok) return false;
        (dynamic_cast<C*>(obj)->*_write)(tv); return true;
    }
    template<void (C::*_write)(const T&)> static bool writeString(Object* obj, const QString& s) {
        STEPCORE_ASSERT_NOABORT(dynamic_cast<const C*>(obj));
        bool ok; T tv = stringToType<T>(s, &ok); if(!ok) return false;
        (dynamic_cast<C*>(obj)->*_write)(tv); return true;
    }
    template<bool (C::*_write)(T)> static bool writeString(Object* obj, const QString& s) {
        STEPCORE_ASSERT_NOABORT(dynamic_cast<const C*>(obj));
        bool ok; T tv = stringToType<T>(s, &ok); if(!ok) return false;
        return (dynamic_cast<C*>(obj)->*_write)(tv);
    }
    template<bool (C::*_write)(const T&)> static bool writeString(Object* obj, const QString& s) {
        STEPCORE_ASSERT_NOABORT(dynamic_cast<const C*>(obj));
        bool ok; T tv = stringToType<T>(s, &ok); if(!ok) return false;
        return (dynamic_cast<C*>(obj)->*_write)(tv);
    }

    static QVariant readNull(const Object* obj) { return QVariant(); }
    static QString readStringNull(const Object* obj) { return QString(); }
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

#define _STEPCORE_PROPERTY_NULL { QString(""), QString(""), 0,0,0,0,0,0 }

#define STEPCORE_META_OBJECT(_className, _description, _flags, __superClasses, __properties) \
    const StepCore::MetaProperty _className::_classProperties[] = { _STEPCORE_PROPERTY_NULL, __properties }; \
    const StepCore::MetaObject*  _className::_superClasses[] = { __superClasses }; \
    const StepCore::MetaObject   _className::_metaObject = { \
        QString(__STRING(_className)), QString(_description), _flags, \
        StepCore::MetaObjectHelper<_className, _flags & StepCore::MetaObject::ABSTRACT>::newObjectHelper, \
        StepCore::MetaObjectHelper<_className, _flags & StepCore::MetaObject::ABSTRACT>::cloneObjectHelper, \
        _superClasses, sizeof(_superClasses)/sizeof(*_superClasses), \
        _classProperties+1, sizeof(_classProperties)/sizeof(*_classProperties)-1, false, 0, 0 };
    
#define STEPCORE_SUPER_CLASS(_className) _className::staticMetaObject(),

#define STEPCORE_PROPERTY_RF(_type, _name, _description, _flags, _read) \
    { __STRING(_name), _description, StepCore::MetaProperty::READABLE | _flags, qMetaTypeId<_type>(), \
      StepCore::MetaPropertyHelper<_thisType, _type>::read<&_thisType::_read>, \
      StepCore::MetaPropertyHelper<_thisType, _type>::writeNull, \
      StepCore::MetaPropertyHelper<_thisType, _type>::readString<&_thisType::_read>, \
      StepCore::MetaPropertyHelper<_thisType, _type>::writeStringNull },

#define STEPCORE_PROPERTY_RWF(_type, _name, _description, _flags, _read, _write) \
    { __STRING(_name), _description, \
      StepCore::MetaProperty::READABLE | StepCore::MetaProperty::WRITABLE | _flags, \
      qMetaTypeId<_type>(), \
      StepCore::MetaPropertyHelper<_thisType, _type>::read<&_thisType::_read>, \
      StepCore::MetaPropertyHelper<_thisType, _type>::write<&_thisType::_write>, \
      StepCore::MetaPropertyHelper<_thisType, _type>::readString<&_thisType::_read>, \
      StepCore::MetaPropertyHelper<_thisType, _type>::writeString<&_thisType::_write> },

#define STEPCORE_PROPERTY_R(_type, _name, _description, _read) \
    STEPCORE_PROPERTY_RF(_type, _name, _description, 0, _read)

#define STEPCORE_PROPERTY_RW(_type, _name, _description, _read, _write) \
    STEPCORE_PROPERTY_RWF(_type, _name, _description, \
        StepCore::MetaProperty::STORED, _read, _write)

#define STEPCORE_PROPERTY_R_D(_type, _name, _description, _read) \
    STEPCORE_PROPERTY_RF(_type, _name, _description, StepCore::MetaProperty::DYNAMIC, _read)

#define STEPCORE_PROPERTY_RW_D(_type, _name, _description, _read, _write) \
    STEPCORE_PROPERTY_RWF(_type, _name, _description, \
        StepCore::MetaProperty::STORED | StepCore::MetaProperty::DYNAMIC, _read, _write)

} // namespace StepCore

#endif

