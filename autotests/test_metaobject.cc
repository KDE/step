/* This file is part of Step
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

#include "test_metaobject.h"
#include "stepcore/object.h"
#include <QString>
#include <QMetaType>
#include <QTest>

struct MetaObjectTestType
{
    int value;
    MetaObjectTestType(int v): value(v) {}
    MetaObjectTestType(): value(0) {}
};
Q_DECLARE_METATYPE( MetaObjectTestType ) 

class MetaObjectTestInterface
{
    STEPCORE_OBJECT(MetaObjectTestInterface)

public:
    virtual ~MetaObjectTestInterface() {}
    double property1() const { return _property1; }
    void setProperty1(double property1) { _property1 = property1; }

protected:
    double _property1;
};

class MetaObjectTestObject: public StepCore::Object, public MetaObjectTestInterface
{
    STEPCORE_OBJECT(MetaObjectTestObject)

public:
    int property2() const { return 2; }

    const MetaObjectTestType& property3() const { return _property3; }
    void setProperty3(const MetaObjectTestType& property3) { _property3 = property3; }

    MetaObjectTestType property4() const { return _property4; }
    bool setProperty4(MetaObjectTestType property4) {
        if(property4.value < 0) return false;
        _property4 = property4; return true;
    }

protected:
    double             _property2;
    MetaObjectTestType _property3;
    MetaObjectTestType _property4;
};

namespace StepCore {
template<> inline QString typeToString(const MetaObjectTestType& v)
{
    return QString::number(v.value);
}

template<> inline MetaObjectTestType stringToType(const QString& s, bool* ok)
{
    return MetaObjectTestType(s.toInt(ok));
}

template<> inline MetaObjectTestType variantToType(const QVariant& v, bool* ok)
{
    if(v.userType() == qMetaTypeId<MetaObjectTestType>()) {
        *ok = true; return v.value<MetaObjectTestType>();
    }
    QVariant vc(v); *ok = vc.convert(QVariant::Int);
    return MetaObjectTestType(vc.value<int>());
}

}

STEPCORE_META_OBJECT(MetaObjectTestInterface, "MetaObjectTestInterface", "TestInterface", StepCore::MetaObject::ABSTRACT,,
        STEPCORE_PROPERTY_RW(double, property1, "property1", "m", "Property1", property1, setProperty1))

STEPCORE_META_OBJECT(MetaObjectTestObject, "MetaObjectTestObject", "TestObject", 0,
        STEPCORE_SUPER_CLASS(StepCore::Object) STEPCORE_SUPER_CLASS(MetaObjectTestInterface),
        STEPCORE_PROPERTY_R (int,   property2, "property2", STEPCORE_UNITS_1, "Property2", property2)
        STEPCORE_PROPERTY_RW(MetaObjectTestType, property3, "property3", STEPCORE_UNITS_NULL, "Property3", property3, setProperty3)
        STEPCORE_PROPERTY_RW(MetaObjectTestType, property4, "property4", STEPCORE_UNITS_NULL, "Property4", property4, setProperty4)
        )

void TestMetaobject::testMetaObject()
{
    /* Abstract class: can't create */
    QVERIFY( MetaObjectTestInterface::staticMetaObject()->isAbstract() );
    QVERIFY( MetaObjectTestInterface::staticMetaObject()->newObject() == NULL );

    /* Normal class: should create */
    QVERIFY( !MetaObjectTestObject::staticMetaObject()->isAbstract() );
    StepCore::Object* object = MetaObjectTestObject::staticMetaObject()->newObject();
    QVERIFY( object != NULL);
    MetaObjectTestObject* testObject = dynamic_cast<MetaObjectTestObject*>(object);
    QVERIFY( testObject != NULL );
    
    QVERIFY( object->metaObject() == MetaObjectTestObject::staticMetaObject() );

    /* Class name */
    const StepCore::MetaObject* metaObject = testObject->metaObject();
    QCOMPARE( QString(metaObject->className()), QString("MetaObjectTestObject") );

    /* Super classes list */
    QCOMPARE( metaObject->superClassCount(), 2 );
    QCOMPARE( QString(metaObject->superClass(0)->className()), QString("Object") );
    QCOMPARE( QString(metaObject->superClass(1)->className()), QString("MetaObjectTestInterface") );

    /* Inheritance */
    QVERIFY( metaObject->inherits(StepCore::Object::staticMetaObject()) );
    QVERIFY( metaObject->inherits(MetaObjectTestInterface::staticMetaObject()) );
    QVERIFY( metaObject->inherits(metaObject) );
    QVERIFY( !MetaObjectTestInterface::staticMetaObject()->inherits(metaObject) );

    QVERIFY( metaObject->inherits("Object") );
    QVERIFY( metaObject->inherits("MetaObjectTestInterface") );
    QVERIFY( metaObject->inherits("MetaObjectTestObject") );
    QVERIFY( !metaObject->inherits("NotClass") );
    QVERIFY( !MetaObjectTestInterface::staticMetaObject()->inherits("MetaObjectTestObject") );

    /* Property count */
    QCOMPARE( metaObject->classPropertyCount(), 3 );
    QCOMPARE( metaObject->propertyCount(), 5 );

    /* Property lookup */
    QVERIFY( metaObject->property("name") == metaObject->property(0) );
    QVERIFY( metaObject->property("property1") == metaObject->property(1) );
    QVERIFY( metaObject->property("property2") == metaObject->property(2) );
    QVERIFY( metaObject->property("property3") == metaObject->property(3) );
    QVERIFY( metaObject->property("property4") == metaObject->property(4) );
    QVERIFY( metaObject->property("notProperty") == NULL );

    const StepCore::MetaProperty* property;

    /* QString property inherited from first base class */ 
    property = testObject->metaObject()->property(0);
    QCOMPARE( QString(property->name()), QString("name") );
    QVERIFY( property->isReadable() );
    QVERIFY( property->isWritable() );
    QVERIFY( property->isStored() );

    QVERIFY( property->writeString(testObject, "test1") );
    QCOMPARE( object->name(), QString("test1") );
    QVERIFY( property->writeVariant(testObject, QVariant(QString("test2"))) );
    QCOMPARE( object->name(), QString("test2") );

    QCOMPARE( property->readString(object), QString("test2") );
    QCOMPARE( property->readVariant(object), QVariant(QString("test2")) );

    /* double property inherited from second base class */
    property = testObject->metaObject()->property(1);
    QCOMPARE( QString(property->name()), QString("property1") );

    QVERIFY( property->writeString(object, "1.1") );
    QCOMPARE( testObject->property1(), 1.1 );
    QVERIFY( property->writeVariant(object, QVariant(2.2)) );
    QCOMPARE( testObject->property1(), 2.2 );
    QCOMPARE( property->readString(object), QString("2.2") );
    QCOMPARE( property->readVariant(object), QVariant(2.2) );

    QVERIFY( !property->writeString(object, "not number") );
    QCOMPARE( testObject->property1(), 2.2 );
    QVERIFY( !property->writeVariant(object, QVariant("not number")) );
    QCOMPARE( testObject->property1(), 2.2 );

    /* double read-only property */
    property = testObject->metaObject()->property(2);
    QCOMPARE( QString(property->name()), QString("property2") );
    QVERIFY( property->isReadable() );
    QVERIFY( !property->isWritable() );
    QVERIFY( !property->isStored() );

    QVERIFY( !property->writeString(object, "10") );
    QCOMPARE( testObject->property2(), 2 );
    QCOMPARE( property->readString(object), QString("2") );

    /* MetaObjectTestType property */
    property = testObject->metaObject()->property(3);
    QCOMPARE( QString(property->name()), QString("property3") );
    
    QVERIFY( property->writeString(object, "2") );
    QCOMPARE( testObject->property3().value, 2 );
    QVERIFY( property->writeVariant(object, 3) );
    QCOMPARE( testObject->property3().value, 3 );

    QCOMPARE( property->readString(object), QString("3") );
    QCOMPARE( property->readVariant(object).value<MetaObjectTestType>().value, 3 );

    /* CloneObject */
    StepCore::Object* clone = object->metaObject()->cloneObject(*object);
    QCOMPARE( clone->name(), object->name() );
    QCOMPARE( dynamic_cast<MetaObjectTestObject*>(clone)->property1(),
              dynamic_cast<MetaObjectTestObject*>(object)->property1());
    QCOMPARE( dynamic_cast<MetaObjectTestObject*>(clone)->property2(),
              dynamic_cast<MetaObjectTestObject*>(object)->property2());
    QCOMPARE( dynamic_cast<MetaObjectTestObject*>(clone)->property3().value,
              dynamic_cast<MetaObjectTestObject*>(object)->property3().value);
    QCOMPARE( dynamic_cast<MetaObjectTestObject*>(clone)->property4().value,
              dynamic_cast<MetaObjectTestObject*>(object)->property4().value);

    delete object;
}

QTEST_MAIN(TestMetaobject)
