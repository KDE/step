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

namespace {

class StepStreamWriter
{
public:
    StepStreamWriter(QIODevice* device): _device(device) {}
    bool writeWorld(const World* world);

protected:
    QString escapeText(const QString& str);
    void saveProperties(const Object* obj, int first, int indent);
    void saveObject(const QString& tag, const Object* obj, int indent);

    QIODevice*   _device;
    QTextStream* _stream;
    QHash<const Object*, int> _ids;
    static const int INDENT = 4;
};

QString StepStreamWriter::escapeText(const QString& str)
{
    QString result = str;
    result.replace("&", "&amp;");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    result.replace("\"", "&quot;");
    return result;
}

void StepStreamWriter::saveProperties(const Object* obj, int first, int indent)
{
    const MetaObject* metaObject = obj->metaObject();
    for(int i = first; i < metaObject->propertyCount(); ++i) {
        const MetaProperty* p = metaObject->property(i);
        if(p->isStored()) {
            *_stream << QString(indent*INDENT, ' ')
                     << "<" << p->name() << ">";

            if(p->userTypeId() == qMetaTypeId<Object*>())
                *_stream << _ids.value(p->readVariant(obj).value<Object*>(), -1);
            else *_stream << escapeText(p->readString(obj));

            *_stream << "</" << p->name() << ">\n";
        }
    }
}

void StepStreamWriter::saveObject(const QString& tag, const Object* obj, int indent)
{
    Q_ASSERT(obj != NULL);

    *_stream << QString(indent*INDENT, ' ') << "<" << tag
             << " class=\"" << QString(obj->metaObject()->className())
             << "\" id=\"" << _ids.value(obj, -1) << "\">\n";

    saveProperties(obj, 0, indent+1);

    if(obj->metaObject()->inherits<Item>()) {
        const ObjectErrors* objErrors = static_cast<const Item*>(obj)->tryGetObjectErrors();
        if(objErrors) saveProperties(objErrors, 1, indent+1);
    }

    if(obj->metaObject()->inherits<ItemGroup>()) {
        const ItemGroup* group = static_cast<const ItemGroup*>(obj);
        *_stream << "\n";
        ItemList::const_iterator end = group->items().end();
        for(ItemList::const_iterator it = group->items().begin(); it != end; ++it) {
            saveObject("item", *it, indent+1);
            *_stream << "\n";
        }
    }

    *_stream << QString(indent*INDENT, ' ') << "</" << tag << ">\n";
}

bool StepStreamWriter::writeWorld(const World* world)
{
    Q_ASSERT(_device->isOpen() && _device->isWritable());
    _stream = new QTextStream(_device);

    int maxid = -1;
    _ids.insert(NULL, ++maxid);
    _ids.insert(world, ++maxid);

    ItemList items = world->allItems();
    const ItemList::const_iterator end0 = items.end();
    for(ItemList::const_iterator it = items.begin(); it != end0; ++it)
        _ids.insert(*it, ++maxid);

    if(world->solver()) _ids.insert(world->solver(), ++maxid);
    if(world->collisionSolver()) _ids.insert(world->collisionSolver(), ++maxid);

    *_stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             << "<!DOCTYPE " << XmlFile::DOCKTYPE << ">\n"
             << "<world xmlns=\"" << XmlFile::NAMESPACE_URI << "\""
             << " version=\"" << XmlFile::VERSION << "\" id=\"1\">\n";

    saveProperties(world, 0, 1);
    *_stream << "\n";

    ItemList::const_iterator end = world->items().end();
    for(ItemList::const_iterator it = world->items().begin(); it != end; ++it) {
        saveObject("item", *it, 1);
        *_stream << "\n";
    }

    if(world->solver()) {
        saveObject("solver", world->solver(), 1);
        *_stream << "\n";
    }

    if(world->collisionSolver()) {
        saveObject("collisionSolver", world->collisionSolver(), 1);
        *_stream << "\n";
    }

    *_stream << "</world>\n";

    delete _stream;
    return true;
}

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
    bool addId(Object* obj, const QString& id);

    enum { START, ITEM, PROPERTY, END } _state;
    World*         _world;
    const Factory* _factory;

    ItemGroup* _parent;
    Object*    _object;
    ObjectErrors*       _objectErrors;
    const MetaProperty* _property;

    typedef QPair<QPair<Object*, const MetaProperty*>, int> Link;
    QHash<int, Object*> _ids;
    QList<Link> _links;

    QString _text;
    QString _errorString;
};

XmlFileHandler::XmlFileHandler(World* world, const Factory* factory)
    : _state(START), _world(world), _factory(factory),
      _parent(NULL), _object(NULL), _objectErrors(NULL), _property(NULL)
{
}

bool XmlFileHandler::addId(Object* obj, const QString& id)
{
    /*
#ifdef __GNUC__
#warning Temporary code
#endif
    if(id.isEmpty()) return true;
    */
    int n = id.trimmed().toInt();
    if(!n) {
        _errorString = QObject::tr("Wrong ID attribute value for %1")
                        .arg(obj->metaObject()->className());
        return false;
    }
    if(_ids.contains(n)) {
        _errorString = QObject::tr("Non-unique ID attribute value for %1")
                        .arg(obj->metaObject()->className());
        return false;
    }

    _ids.insert(n, _object);
    return true;
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

            if(!addId(_object, attributes.value("id"))) return false;

        } else {
            _errorString = QObject::tr("The file is not a StepCoreXML file.");
            return false;
        }
        break;

    case ITEM:
        if(qName == "item" && _object->metaObject()->inherits<ItemGroup>()) {
            _parent = static_cast<ItemGroup*>(_object);

            QString className = attributes.value("class");
            Item* item = _factory->newItem(className);
            if(item == NULL) {
                _errorString = QObject::tr("Unknown item type \"%1\"").arg(className);
                return false;
            }

            _parent->addItem(item);
            _object = item;

            if(!addId(_object, attributes.value("id"))) return false;

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

            if(!addId(_object, attributes.value("id"))) return false;

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

            if(!addId(_object, attributes.value("id"))) return false;

            break;

        }

        _property = _object->metaObject()->property(qName);
        if(!_property && _object->metaObject()->inherits<Item>()) {
            const MetaObject* objErrors = _factory->metaObject(_object->metaObject()->className()+"Errors");
            if(objErrors) {
                _property = objErrors->property(qName);
                if(_property && _property->isStored())
                    _objectErrors = static_cast<Item*>(_object)->objectErrors();
            }
        }

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
        if(_property->userTypeId() == qMetaTypeId<Object*>()) {
            /*
#ifdef __GNUC__
#warning Temporary code
#endif
            if(!_text.trimmed()[0].isDigit()) {
                Object* o = _world->object(_text);
                qDebug("deprecated link to %s (%p)", _text.toLatin1().constData(), o);
                QVariant obj = QVariant::fromValue(o);
                _property->writeVariant(_object, obj);
            } else*/ {
                int n = _text.trimmed().toInt();
                _links.push_back(qMakePair(qMakePair(
                        static_cast<Object*>(_objectErrors ? _objectErrors : _object),
                        _property), n));
            }
        }
        else if(!_property->writeString(_objectErrors ? _objectErrors : _object, _text)) {
            _errorString = QObject::tr("Property \"%1\" of \"%2\" has illegal value")
                                .arg(qName, _object->metaObject()->className());
            return false;
        }
        _objectErrors = NULL;
        _state = ITEM;
        break;

    case ITEM:
        if(_parent == NULL) {
            STEPCORE_ASSERT_NOABORT(_object == _world);
            _state = END;
        } else {
            STEPCORE_ASSERT_NOABORT(_parent->metaObject()->inherits<Item>());
            Item* item = static_cast<Item*>(_parent);
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

    // Connect links
    foreach(const Link& link, _links) {
        QVariant target = QVariant::fromValue(_ids.value(link.second, NULL));
        if(!link.first.second->writeVariant(link.first.first, target)) {
            _errorString = QObject::tr("Property \"%1\" of \"%2\" has illegal value")
                    .arg(link.first.second->name(), link.first.first->metaObject()->className());
            return false;
        }
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

bool XmlFile::save(const World* world)
{
    if(!_device->isOpen() || !_device-> isWritable()) {
        _errorString = QObject::tr("File is not writeble.");
        return false;
    }

    StepStreamWriter writer(_device);
    return writer.writeWorld(world);
}

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

