/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "object.h"
#include <cstring>
#include <QCoreApplication>

namespace StepCore {

STEPCORE_META_OBJECT(Object, QT_TRANSLATE_NOOP("ObjectClass", "Object"), QT_TRANSLATE_NOOP("ObjectDescription", "Object"), MetaObject::ABSTRACT,,
        STEPCORE_PROPERTY_RW(QString, name, QT_TRANSLATE_NOOP("PropertyName", "name"), STEPCORE_UNITS_NULL, QT_TRANSLATE_NOOP("PropertyDescription", "Object name"), name, setName))

int MetaObject::s_classIdCount = 0;

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

    // properties
    _allPropertyCount = classPropertyCount();
    for(int i=0; i<superClassCount(); ++i) {
        _allPropertyCount += superClass(i)->propertyCount();
    }

    _allProperties = new const MetaProperty*[_allPropertyCount];
    copyProperties(_allProperties);

    // classId and super classes
    _classId = s_classIdCount++; // all super classes is already registered
    _allSuperClassIds.fill(false, s_classIdCount);
    _allSuperClassIds.setBit(_classId);
    for(int i=0; i<superClassCount(); ++i) {
        _allSuperClassIds |= superClass(i)->_allSuperClassIds;
    }

    // strings
    _classNameTr = QCoreApplication::translate("ObjectClass", _className.toUtf8().constData());
    _descriptionTr = QCoreApplication::translate("ObjectDescription", _description.toUtf8().constData());

    _initialized = true;
}

bool MetaObject::inherits(const MetaObject* obj) const
{
    if(!_initialized) init();
    int objClassId = obj->classId();
    if(objClassId == _classId) return true;
    else if(objClassId > _classId) return false;
    return _allSuperClassIds.testBit(objClassId);
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

void MetaProperty::init() const
{
    _nameTr = QCoreApplication::translate("PropertyName", _name.toUtf8().constData());
    _unitsTr = QCoreApplication::translate("Units", _units.toUtf8().constData());
    _descriptionTr = QCoreApplication::translate("PropertyDescription", _description.toUtf8().constData());

    _initialized = true;
}

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
    return nullptr;
}

} // namespace StepCore

