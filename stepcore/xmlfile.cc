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

#include "object.h"
#include "world.h"
#include "solver.h"
#include "collisionsolver.h"
#include "factory.h"

#include <QTextStream>
#include <QMetaProperty>
#include <QXmlDefaultHandler>

namespace StepCore {

const char* XmlFile::DOCKTYPE = "StepCoreXML";
const char* XmlFile::NAMESPACE_URI = "http://edu.kde.org/step/StepCoreXML";
const char* XmlFile::VERSION = "1.0";
const int   XmlFile::INDENT = 4;

XmlFile::XmlFile(QIODevice* device)
    : _device(device)
{
}

QString XmlFile::errorString() const
{
    return _errorString;
}

bool XmlFile::save(const World* world)
{
    if(!_device->isOpen() || !_device-> isWritable()) {
        _errorString = QObject::tr("File is not writeble.");
        return false;
    }

    QTextStream stream(_device);

    stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
           << "<!DOCTYPE " << DOCKTYPE << ">\n"
           << "<world xmlns=\"" << NAMESPACE_URI << "\""
           << " version=\"" << VERSION << "\">\n";

    saveProperties(1, world, stream);
    stream << "\n";

    ItemList::const_iterator end = world->items().end();
    for(ItemList::const_iterator it = world->items().begin(); it != end; ++it) {
        saveObject(1, "item", *it, stream);
        stream << "\n";
    }

    saveObject(1, "solver", world->solver(), stream);
    saveObject(1, "collisionSolver", world->collisionSolver(), stream);
    stream << "\n";
    stream << "</world>\n";

    return true;
}

QString XmlFile::escapeText(const QString& str)
{
    QString result = str;
    result.replace("&", "&amp;");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    result.replace("\"", "&quot;");
    return result;
}

void XmlFile::saveProperties(int indent, const Object* obj, QTextStream& stream)
{
    const MetaObject* metaObject = obj->metaObject();
    for(int i = 0; i < metaObject->propertyCount(); ++i) {
        const MetaProperty* p = metaObject->property(i);
        if(p->isStored()) {
            stream << QString(indent*INDENT, ' ')
                   << "<" << p->name() << ">"
                   << escapeText(p->readString(obj))
                   << "</" << p->name() << ">\n";
        }
    }
}

void XmlFile::saveObject(int indent, const QString& tag, const Object* obj, QTextStream& stream)
{
    if(obj == NULL) return;
    stream << QString(indent*INDENT, ' ') << "<" << tag
           << " class=\"" << QString(obj->metaObject()->className()) << "\">\n";
    saveProperties(indent+1, obj, stream);

    const ItemGroup* group = dynamic_cast<const ItemGroup*>(obj);
    if(group) {
        stream << "\n";
        ItemList::const_iterator end = group->items().end();
        for(ItemList::const_iterator it = group->items().begin(); it != end; ++it) {
            saveObject(indent+1, "item", *it, stream);
            stream << "\n";
        }
    }

    stream << QString(indent*INDENT, ' ') << "</" << tag << ">\n";
}

namespace {

class XmlFileHandler: public QXmlDefaultHandler
{
public:
    XmlFileHandler(World* world, const Factory* factory);

    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName);
    bool characters(const QString &str);
    bool fatalError(const QXmlParseException &exception);
    bool endDocument();
    QString errorString() const;

protected:
    //enum { START, WORLD, WORLD_PROPERTY, ITEM, ITEM_PROPERTY, END } _state;
    enum { START, ITEM, PROPERTY, END } _state;
    World*         _world;
    const Factory* _factory;

    ItemGroup* _parent;
    Object*    _object;
    const MetaProperty* _property;

    QString _text;
    QString _errorString;
};

XmlFileHandler::XmlFileHandler(World* world, const Factory* factory)
    : _state(START), _world(world), _factory(factory),
      _parent(NULL), _object(NULL), _property(NULL)
{
}

bool XmlFileHandler::startElement(const QString &namespaceURI, const QString &,
                  const QString &qName, const QXmlAttributes &attributes)
{
    if(namespaceURI != XmlFile::NAMESPACE_URI) return true; // XXX: is it correct behaviour ?

    switch(_state) {
    case START:
        if(qName == "world") {
            _parent = NULL;
            _object = _world;
            _state = ITEM;
        } else {
            _errorString = QObject::tr("The file is not a StepCoreXML file.");
            return false;
        }
        break;

    case ITEM:
        if(qName == "item" && dynamic_cast<ItemGroup*>(_object)) {
            _parent = dynamic_cast<ItemGroup*>(_object);

            Item* item = _factory->newItem(attributes.value("class")); 
            if(item == NULL) {
                _errorString = QObject::tr("Unknown item type \"%1\"").arg(attributes.value("class"));
                return false;
            }

            _parent->addItem(item);
            _object = item;
            break;

        } else if(_object == _world && qName == "solver") {
            Solver* solver = _factory->newSolver(attributes.value("class")); 
            if(solver == NULL) {
                _errorString = QObject::tr("Unknown solver type \"%1\"").arg(attributes.value("class"));
                return false;
            }

            _world->setSolver(solver);
            _object = solver;
            _parent = _world;
            break;

        } else if(_object == _world && qName == "collisionSolver") {
            CollisionSolver* collisionSolver = _factory->newCollisionSolver(attributes.value("class")); 
            if(collisionSolver == NULL) {
                _errorString = QObject::tr("Unknown collisionSolver type \"%1\"").arg(attributes.value("class"));
                return false;
            }

            _world->setCollisionSolver(collisionSolver);
            _object = collisionSolver;
            _parent = _world;
            break;

        }

        _property = _object->metaObject()->property(qName.toAscii().constData());
        if(!_property || !_property->isStored()) {
            _errorString = QObject::tr("Item \"%1\" has no stored property named \"%2\"")
                                .arg(QString(_object->metaObject()->className())).arg(qName);
            return false;
        }
        _text.clear();
        _state = PROPERTY;
        break;

    case PROPERTY:
    default:
        _errorString = QObject::tr("Unexpected tag \"%1\"").arg(qName);
        return false;
    }

    return true;
}

bool XmlFileHandler::endElement(const QString &namespaceURI, const QString &,
                const QString &qName)
{
    if(namespaceURI != XmlFile::NAMESPACE_URI) return true; // XXX: is it correct behaviour ?

    switch(_state) {
    case PROPERTY:
        if(!_property->writeString(_object, _text)) {
            _errorString = QObject::tr("Property \"%1\" has illegal value").arg(qName);
            return false;
        }
        _state = ITEM;
        break;

    case ITEM:
        if(_parent == NULL) {
            STEPCORE_ASSERT_NOABORT(_object == _world);
            _state = END;
        } else {
            Item* item = dynamic_cast<Item*>(_parent);
            STEPCORE_ASSERT_NOABORT(item != NULL);
            _object = _parent;
            _parent = item->group();
        }
        break;

    default:
        STEPCORE_ASSERT_NOABORT(false);
    }

    return true;
}

bool XmlFileHandler::characters(const QString &str)
{
    if(_state == PROPERTY) _text += str;
    return true;
}

bool XmlFileHandler::endDocument()
{
    if(_state != END) {
        _errorString = QObject::tr("\"world\" tag not found");
        return false;
    }
    return true;
}

bool XmlFileHandler::fatalError(const QXmlParseException &exception)
{
    _errorString = QObject::tr("Error parsing file at line %1: %2")
                        .arg(exception.lineNumber()).arg(exception.message());
    return false;
}

QString XmlFileHandler::errorString() const
{
    return _errorString;
}

} // namespace

bool XmlFile::load(World* world, const Factory* factory)
{
    XmlFileHandler handler(world, factory);
    QXmlInputSource source(_device);
    QXmlSimpleReader reader;

    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);
    if(reader.parse(source)) return true;
    _errorString = handler.errorString();
    return false;
}

} // namespace StepCore

#endif //STEPCORE_WITH_QT

