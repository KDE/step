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

#include "xmlfile.h"

#ifdef STEPCORE_WITH_QT

#include "world.h"
#include "solver.h"
#include "collisionsolver.h"
#include "constraintsolver.h"
#include "factory.h"

#include <QDomDocument>
#include <QXmlStreamWriter>

namespace StepCore {

const char* XmlFile::DOCTYPE = "<!DOCTYPE StepCoreXML>";
const char* XmlFile::NAMESPACE_URI = "https://edu.kde.org/step/StepCoreXML";
const char* XmlFile::VERSION = "1.0";

namespace {

class StepStreamWriter
{
public:
    StepStreamWriter(QIODevice* device);
    bool writeWorld(const World* world);

protected:
    void saveProperties(const Object* obj, int first);
    void saveObject(const QString& tag, const Object* obj);

    QXmlStreamWriter _writer;
    QIODevice* _device;
    QHash<const Object*, int> _ids;
    static const int INDENT = 4;
};

StepStreamWriter::StepStreamWriter(QIODevice* device) : _device(device)
{
    _writer.setAutoFormatting(true);
    _writer.setAutoFormattingIndent(INDENT);
}

void StepStreamWriter::saveProperties(const Object* obj, int first)
{
    const MetaObject* metaObject = obj->metaObject();
    for(int i = first; i < metaObject->propertyCount(); ++i) {
        const MetaProperty* p = metaObject->property(i);
        if(p->isStored()) {
            if(p->userTypeId() == qMetaTypeId<Object*>()) {
                int id = _ids.value(p->readVariant(obj).value<Object*>(), -1);
                _writer.writeTextElement(p->name(), QString::number(id));
            }
            else {
                _writer.writeTextElement(p->name(), p->readString(obj));
            }
        }
    }
}

void StepStreamWriter::saveObject(const QString& tag, const Object* obj)
{
    Q_ASSERT(obj != NULL);
    
    _writer.writeStartElement(tag);
    _writer.writeAttribute(QStringLiteral("class"), obj->metaObject()->className());
    _writer.writeAttribute(QStringLiteral("id"), QString::number(_ids.value(obj, -1)));

    saveProperties(obj, 0);

    if(obj->metaObject()->inherits<Item>()) {
        const ObjectErrors* objErrors = static_cast<const Item*>(obj)->tryGetObjectErrors();
        if(objErrors) saveProperties(objErrors, 1);
    }

    if(obj->metaObject()->inherits<ItemGroup>()) {
        const ItemGroup* group = static_cast<const ItemGroup*>(obj);
        ItemList::const_iterator end = group->items().end();
        for(ItemList::const_iterator it = group->items().begin(); it != end; ++it) {
            saveObject(QStringLiteral("item"), *it);
        }
    }
    
    _writer.writeEndElement();
}

bool StepStreamWriter::writeWorld(const World* world)
{
    Q_ASSERT(_device->isOpen() && _device->isWritable());
    _writer.setDevice(_device);

    int maxid = -1;
    _ids.insert(NULL, ++maxid);
    _ids.insert(world, ++maxid);

    ItemList items = world->allItems();
    const ItemList::const_iterator end0 = items.end();
    for(ItemList::const_iterator it = items.begin(); it != end0; ++it)
        _ids.insert(*it, ++maxid);

    if(world->solver()) _ids.insert(world->solver(), ++maxid);
    if(world->collisionSolver()) _ids.insert(world->collisionSolver(), ++maxid);
    if(world->constraintSolver()) _ids.insert(world->constraintSolver(), ++maxid);

    _writer.writeStartDocument();
    _writer.writeDTD(XmlFile::DOCTYPE);
    _writer.writeStartElement(QStringLiteral("world"));
    _writer.writeAttribute(QStringLiteral("xmlns"), XmlFile::NAMESPACE_URI);
    _writer.writeAttribute(QStringLiteral("version"), XmlFile::VERSION);
    _writer.writeAttribute(QStringLiteral("id"), QStringLiteral("1"));

    saveProperties(world, 0);

    ItemList::const_iterator end = world->items().end();
    for(ItemList::const_iterator it = world->items().begin(); it != end; ++it) {
        saveObject(QStringLiteral("item"), *it);
    }

    if(world->solver()) {
        saveObject(QStringLiteral("solver"), world->solver());
    }

    if(world->collisionSolver()) {
        saveObject(QStringLiteral("collisionSolver"), world->collisionSolver());
    }

    if(world->constraintSolver()) {
        saveObject(QStringLiteral("constraintSolver"), world->constraintSolver());
    }
    
    _writer.writeEndElement();
    _writer.writeEndDocument();
    
    return true;
}

class StepDomDocument
{
public:
    StepDomDocument(World* world, const Factory* factory);
    
    bool parse(QIODevice* device);
    
    const QString& errorMsg() const { return _errorMsg; }
    
private:
    typedef QPair<QPair<Object*, const MetaProperty*>, int> Link;
    
    Item* createItem(const QDomElement& element);
    Solver* createSolver(const QDomElement& element);
    CollisionSolver* createCollisionSolver(const QDomElement& element);
    ConstraintSolver* createConstraintSolver(const QDomElement& element);
    bool parseWorld(const QDomElement& element);
    bool parseItems(ItemGroup* parent, const QDomElement& element);
    bool parseObject(Object* object, const QDomElement& element);
    bool parseProperties(Object* object, const QDomElement& parent);
    bool connectLinks();
    
    World* _world;
    const Factory* _factory;
    QDomDocument _document;
    QString _errorMsg;
    int _errorLine;
    int _errorCount;
    QString _version;
    QHash<int, Object*> _ids;
    QList<Link> _links;
};

StepDomDocument::StepDomDocument(World* world, const StepCore::Factory* factory) :
    _world(world), _factory(factory), _errorLine(0), _errorCount(0)
{
}

bool StepDomDocument::parse(QIODevice* device)
{
    if (!_document.setContent(device, &_errorMsg, &_errorLine, &_errorCount)) {
        return false;
    }
    
    QDomElement worldElement = _document.firstChildElement(QStringLiteral("world"));
    if (worldElement.isNull()) {
        _errorMsg = QObject::tr("The file is not a StepCoreXML file.");
        return false;
    }
    
    return parseWorld(worldElement);
}

bool StepDomDocument::parseWorld(const QDomElement& world)
{
    _version = world.attribute(QStringLiteral("version"), QStringLiteral("1.0"));
    
    if (!parseObject(_world, world)) return false;
    if (!parseItems(_world, world)) return false;
    
    QDomElement solverElement = world.firstChildElement(QStringLiteral("solver"));
    if (!solverElement.isNull()) {
        Solver *solver = createSolver(solverElement);
        if (!solver) return false;
        
        _world->setSolver(solver);
    }
    
    QDomElement collisionSolverElement = world.firstChildElement(QStringLiteral("collisionSolver"));
    if (!collisionSolverElement.isNull()) {
        CollisionSolver *solver = createCollisionSolver(collisionSolverElement);
        if (!solver) return false;
        
        _world->setCollisionSolver(solver);
    }
    
    QDomElement constraintSolverElement = world.firstChildElement(QStringLiteral("constraintSolver"));
    if (!constraintSolverElement.isNull()) {
        ConstraintSolver *solver = createConstraintSolver(constraintSolverElement);
        if (!solver) return false;
        
        _world->setConstraintSolver(solver);
    }
    
    return connectLinks();
}

Item* StepDomDocument::createItem(const QDomElement& element)
{
    QString className = element.attribute(QStringLiteral("class"));
    QScopedPointer<Item> item(_factory->newItem(className));
    if (!item) {
        _errorMsg = QObject::tr("Unknown item type \"%1\"").arg(className);
        return 0;
    }
    
    if (!parseObject(item.data(), element)) return 0;
    ObjectErrors *objErrors = item->objectErrors();
    if (objErrors && !parseProperties(objErrors, element)) return 0;
    
    if (item->metaObject()->inherits("ItemGroup")) {
        ItemGroup *group = static_cast<ItemGroup*>(item.data());
        if (!parseItems(group, element)) return 0;
    }

    return item.take();
}

Solver* StepDomDocument::createSolver(const QDomElement& element)
{
    QString className = element.attribute(QStringLiteral("class"));
    QScopedPointer<Solver> solver(_factory->newSolver(className));
    if (!solver) {
        _errorMsg = QObject::tr("Unknown solver type \"%1\"").arg(className);
        return 0;
    }
    
    if (!parseObject(solver.data(), element)) return 0;
    
    return solver.take();
}

CollisionSolver* StepDomDocument::createCollisionSolver(const QDomElement& element)
{
    QString className = element.attribute(QStringLiteral("class"));
    QScopedPointer<CollisionSolver> solver(_factory->newCollisionSolver(className));
    if (!solver) {
        _errorMsg = QObject::tr("Unknown collisionSolver type \"%1\"").arg(className);
        return 0;
    }
    
    if (!parseObject(solver.data(), element)) return 0;
    
    return solver.take();
}

ConstraintSolver* StepDomDocument::createConstraintSolver(const QDomElement& element)
{
    QString className = element.attribute(QStringLiteral("class"));
    QScopedPointer<ConstraintSolver> solver(_factory->newConstraintSolver(className));
    if (!solver) {
        _errorMsg = QObject::tr("Unknown constraint solver type \"%1\"").arg(className);
        return 0;
    }
    
    if (!parseObject(solver.data(), element)) return 0;
    
    return solver.take();
}

bool StepDomDocument::parseItems(ItemGroup* parent, const QDomElement& element)
{
    QDomElement itemElement = element.firstChildElement(QStringLiteral("item"));
    while (!itemElement.isNull()) {
        Item *item = createItem(itemElement);
        if (!item) return false;
        
        parent->addItem(item);
        itemElement = itemElement.nextSiblingElement(QStringLiteral("item"));
    }
    
    return true;
}

bool StepDomDocument::parseObject(Object* object, const QDomElement& element)
{
    int n = element.attribute(QStringLiteral("id")).trimmed().toInt();
    
    if (!n) {
        _errorMsg = QObject::tr("Wrong ID attribute value for %1")
        .arg(object->metaObject()->className());
        return false;
    }
    if (_ids.contains(n)) {
        _errorMsg = QObject::tr("Non-unique ID attribute value for %1")
        .arg(object->metaObject()->className());
        return false;
    }
    
    _ids.insert(n, object);
    
    return parseProperties(object, element);
}

bool StepDomDocument::parseProperties(Object* object, const QDomElement& parent)
{
    int properties = object->metaObject()->propertyCount();
    for (int n = 0; n < properties; ++n) {
        const MetaProperty* property = object->metaObject()->property(n);
        
        if (!property->isStored()) continue;
        
        QString name = property->name();
        QDomElement propertyElement = parent.firstChildElement(name);
        if (propertyElement.isNull()) continue;
        
        QString text = propertyElement.text();
        if (property->userTypeId() == qMetaTypeId<Object*>()) {
            int n = text.trimmed().toInt();
            _links.push_back(qMakePair(qMakePair(object, property), n));
        }
        else if (!property->writeString(object, text)) {
            _errorMsg = QObject::tr("Property \"%1\" of \"%2\" has illegal value")
                .arg(name, object->metaObject()->className());
            return false;
        }
    }
    
    return true;
}

bool StepDomDocument::connectLinks()
{
    foreach (const Link& link, _links) {
        QVariant target = QVariant::fromValue(_ids.value(link.second, 0));
        if (!link.first.second->writeVariant(link.first.first, target)) {
            _errorMsg = QObject::tr("Property \"%1\" of \"%2\" has illegal value")
                .arg(link.first.second->name(), link.first.first->metaObject()->className());
            return false;
        }
    }
    
    return true;
}
} // namespace

bool XmlFile::save(const World* world)
{
    if(!_device->isOpen() || !_device-> isWritable()) {
        _errorString = QObject::tr("File is not writable.");
        return false;
    }

    StepStreamWriter writer(_device);
    return writer.writeWorld(world);
}

bool XmlFile::load(World* world, const Factory* factory)
{
    StepDomDocument document(world, factory);
    if (!document.parse(_device)) {
        _errorString = document.errorMsg();
        return false;
    }
    
    return true;
}
} // namespace StepCore

#endif //STEPCORE_WITH_QT

