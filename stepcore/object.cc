#include "object.h"
#include <cstring>

namespace StepCore {

STEPCORE_META_OBJECT(Object, "Object", MetaObject::ABSTRACT,,
        STEPCORE_PROPERTY_RW(QString, name, "Object name", name, setName))

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
    if(std::strcmp(className(), name) == 0) return true;
    for(int i=superClassCount()-1; i>=0; --i) {
        if(std::strcmp(superClass(i)->className(), name) == 0) return true;
    }
    return false;
}

int MetaObject::propertyCount() const
{
    int count = 0;
    for(int i=0; i<superClassCount(); ++i)
        count += superClass(i)->propertyCount();
    count += classPropertyCount();
    return count;
}

const MetaProperty* MetaObject::property(int n) const
{
    for(int i=0; i<superClassCount(); ++i) {
        const MetaProperty* pr = superClass(i)->property(n);
        if(pr) return pr;
        n -= superClass(i)->propertyCount();
    }
    if(n < classPropertyCount()) return &_classProperties[n];
    else return NULL;
}

const MetaProperty* MetaObject::property(const char* name) const
{
    for(int i=0; i<classPropertyCount(); ++i) {
        if(std::strcmp(classProperty(i)->name(), name) == 0)
            return classProperty(i);
    }
    for(int i=superClassCount()-1; i>=0; --i) {
        const MetaProperty* pr = superClass(i)->property(name);
        if(pr) return pr;
    }
    return NULL;
}

} // namespace StepCore

