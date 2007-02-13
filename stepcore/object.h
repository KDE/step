#ifndef STEPCORE_OBJECT_H
#define STEPCORE_OBJECT_H

/** \file object.h
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

/** Object */
class Object
{
    STEPCORE_OBJECT(Object)

public:
    virtual ~Object() {}

    const QString& name() const { return _name; }
    void setName(const QString& name) { _name = name; }

protected:
    QString _name;
};

/** MetaProperty */
class MetaProperty
{
public:
    enum { READABLE = 1, WRITABLE = 2, STORED = 4, };

public:
    const char* name() const { return _name; }
    const char* description() const { return _description; }
    int flags() const { return _flags; }

    int userTypeId() const { return _userTypeId; }
    QVariant readVariant(const Object* obj) const { return _readVariant(obj); }
    bool writeVariant(Object* obj, const QVariant& v) const { return _writeVariant(obj, v); }

    QString readString(const Object* obj) const { return _readString(obj); }
    bool writeString(Object* obj, const QString& s) const { return _writeString(obj, s); }

    bool isReadable() const { return _flags & READABLE; }
    bool isWritable() const { return _flags & WRITABLE; }
    bool isStored() const { return _flags & STORED; }

public:
    const char* const _name;
    const char* const _description;
    const int _flags;
    const int _userTypeId;
    QVariant (*const _readVariant)(const Object* obj);
    bool (*const _writeVariant)(Object* obj, const QVariant& v);
    QString (*const _readString)(const Object* obj);
    bool (*const _writeString)(Object* obj, const QString& v);
};

/** MetaObject */
class MetaObject
{
public:
    enum { ABSTRACT = 1 };

public:
    const char* className() const { return _className; }
    const char* description() const { return _description; }

    bool isAbstract() const { return _flags & ABSTRACT; }
    Object* newObject() const { return _newObject(); }

    int superClassCount() const { return _superClassCount; }
    const MetaObject* superClass(int n) const { return _superClasses[n]; }
    bool inherits(const MetaObject* obj) const;
    bool inherits(const char* name) const;

    int classPropertyCount() const { return _classPropertyCount; }
    const MetaProperty* classProperty(int n) const { return &_classProperties[n]; }

    int propertyCount() const; // TODO caching
    const MetaProperty* property(int n) const; // TODO caching
    const MetaProperty* property(const char* name) const;

public:
    const char* const _className;
    const char* const _description;
    const int         _flags;
    Object* (*const _newObject)();

    const MetaObject**  _superClasses;
    const int           _superClassCount;
    const MetaProperty* _classProperties;
    const int           _classPropertyCount;
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
struct PropertyHelper {

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
    static bool writeNull(Object* obj, const QVariant& v) { return false; }
    static bool writeStringNull(Object* obj, const QString& s) { return false; }
};

template<typename Class, int N>
struct newObjectHelperStruct {
    static Object* newObjectHelper() { return new Class(); }
};

template<class Class>
struct newObjectHelperStruct<Class, MetaObject::ABSTRACT> {
    static Object* newObjectHelper() { return NULL; }
};

#define STEPCORE_META_OBJECT(_className, _description, _flags, __superClasses, __properties) \
    const StepCore::MetaProperty _className::_classProperties[] = { __properties }; \
    const StepCore::MetaObject*  _className::_superClasses[] = { __superClasses }; \
    const StepCore::MetaObject   _className::_metaObject = { \
        __STRING(_className), _description, _flags, \
        StepCore::newObjectHelperStruct<_className, _flags & StepCore::MetaObject::ABSTRACT>::newObjectHelper, \
        _superClasses, sizeof(_superClasses)/sizeof(*_superClasses), \
        _classProperties, sizeof(_classProperties)/sizeof(*_classProperties) };
    
#define STEPCORE_SUPER_CLASS(_className) _className::staticMetaObject(),

#define STEPCORE_PROPERTY_R(_type, _name, _description, _read) \
    { __STRING(_name), _description, StepCore::MetaProperty::READABLE, qMetaTypeId<_type>(),  \
      StepCore::PropertyHelper<_thisType, _type>::read<&_thisType::_read>, \
      StepCore::PropertyHelper<_thisType, _type>::writeNull, \
      StepCore::PropertyHelper<_thisType, _type>::readString<&_thisType::_read>, \
      StepCore::PropertyHelper<_thisType, _type>::writeStringNull },

#define STEPCORE_PROPERTY_RWS(_type, _name, _description, _read, _write) \
    { __STRING(_name), _description, \
      StepCore::MetaProperty::READABLE | StepCore::MetaProperty::WRITABLE | StepCore::MetaProperty::STORED, \
      qMetaTypeId<_type>(), \
      StepCore::PropertyHelper<_thisType, _type>::read<&_thisType::_read>, \
      StepCore::PropertyHelper<_thisType, _type>::write<&_thisType::_write>, \
      StepCore::PropertyHelper<_thisType, _type>::readString<&_thisType::_read>, \
      StepCore::PropertyHelper<_thisType, _type>::writeString<&_thisType::_write> },

} // namespace StepCore

#endif

