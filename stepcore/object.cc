#include "object.h"
#include <cstring>

namespace StepCore {

STEPCORE_META_OBJECT(Object, "Object", MetaObject::ABSTRACT,,
        STEPCORE_PROPERTY_RW(QString, name, "Object name", name, setName))

void MetaObject::copyProperties(const MetaProperty** dest) const
{
    int c = 0;
    for(int i=0; i<superClassCount(); ++i) {
        superClass(i)->copyProperties(dest + c);
        c += superClass(i)->propertyCount();
    }
    for(int i=0; i<_classPropertyCount; ++i) {
        dest[c+i] = _classProperties + i;
    }
}

void MetaObject::init() const
{
    if(_initialized) return;
    for(int i=0; i<superClassCount(); ++i) {
        _allPropertyCount += superClass(i)->propertyCount();
    }
    _allPropertyCount += classPropertyCount();

    _allProperties = new const MetaProperty*[_allPropertyCount];
    copyProperties(_allProperties);

    _initialized = true;
}

bool MetaObject::inherits(const MetaObject* obj) const
{
    if(obj == this) return true;
    for(int i=superClassCount()-1; i>=0; --i) {
        if(superClass(i)->inherits(obj)) return true;
    }
    return false;
}

bool MetaObject::inherits(const char* name) const
{
    //if(std::strcmp(className(), name) == 0) return true;
    if(className() == name) return true;
    for(int i=superClassCount()-1; i>=0; --i) {
        if(superClass(i)->inherits(name)) return true;
    }
    return false;
}

/*
int MetaObject::propertyCount() const
{
    if(_allPropertyCount >= 0) return _allPropertyCount;

    _allPropertyCount = 0;
    for(int i=0; i<superClassCount(); ++i)
        _allPropertyCount += superClass(i)->propertyCount();
    _allPropertyCount += classPropertyCount();
    return _allPropertyCount;
}*/

/*
const MetaProperty* MetaObject::property(int n) const
{
    for(int i=0; i<superClassCount(); ++i) {
        const MetaProperty* pr = superClass(i)->property(n);
        if(pr) return pr;
        n -= superClass(i)->propertyCount();
    }
    if(n < classPropertyCount()) return &_classProperties[n];
    else return NULL;
}*/

const MetaProperty* MetaObject::property(const QString& name) const
{
    if(!_initialized) init();
    for(int i=0; i<_allPropertyCount; ++i) {
        //if(std::strcmp(_allProperties[i]->name(), name) == 0)
        if(_allProperties[i]->name() == name)
            return _allProperties[i];
    }
    /*
    for(int i=0; i<classPropertyCount(); ++i) {
        if(std::strcmp(classProperty(i)->name(), name) == 0)
            return classProperty(i);
    }*/
    /*
    for(int i=superClassCount()-1; i>=0; --i) {
        const MetaProperty* pr = superClass(i)->property(name);
        if(pr) return pr;
    }*/
    return NULL;
}

} // namespace StepCore

