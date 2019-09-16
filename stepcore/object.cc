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
    _descriptionTr = QCoreApplication::translate(NULL, _description.toUtf8().constData());

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
    _descriptionTr = QCoreApplication::translate(NULL, _description.toUtf8().constData());

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
    return NULL;
}

} // namespace StepCore

