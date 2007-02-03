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
#include "factory.h"
#include <QTextStream>
#include <QMetaProperty>
#include <QXmlDefaultHandler>

namespace StepCore {

const char* XmlFile::DOCKTYPE = "StepCoreXML";
const char* XmlFile::NAMESPACE_URI = "http://quantum.dgap.mipt.ru/StepCoreXML";
const char* XmlFile::VERSION = "1.0";
const int   XmlFile::INDENT = 4;

XmlFile::XmlFile(QIODevice* device, const Factory* factory)
    : _device(device), _factory(factory)
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

    for(World::ItemList::const_iterator item  = world->items().begin();
                                           item != world->items().end(); ++item) {
        saveObject(1, "item", *item, stream);
        stream << "\n";
    }

    saveObject(1, "solver", world->solver(), stream);
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
    return result;
}

void XmlFile::saveProperties(int indent, const QObject* obj, QTextStream& stream)
{
    for(int i = 0; i < obj->metaObject()->propertyCount(); ++i) {
        QMetaProperty p = obj->metaObject()->property(i);
        if(!p.isStored(obj)) continue;
        stream << QString(indent*INDENT, ' ')
               << "<" << p.name() << ">"
               << _factory->variantToString(p.read(obj))
               << "</" << p.name() << ">\n";
    }
}

void XmlFile::saveObject(int indent, const QString& tag, const QObject* obj, QTextStream& stream)
{
    if(obj == NULL) return;
    stream << QString(indent*INDENT, ' ') << "<" << tag
           << " class=\"" << QString(obj->metaObject()->className()).remove("StepCore::") << "\">\n";
    saveProperties(indent+1, obj, stream);
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
    QString errorString() const;

protected:
    enum { START, WORLD, WORLD_PROPERTY, ITEM, ITEM_PROPERTY, END } _state;
    World*         _world;
    const Factory* _factory;

    QObject* _item;
    int      _propertyIdx;
    QString  _text;
    QString  _errorString;
};

XmlFileHandler::XmlFileHandler(World* world, const Factory* factory)
    : _state(START), _world(world), _factory(factory), _item(NULL)
{
}

bool XmlFileHandler::startElement(const QString &namespaceURI, const QString &localName,
                  const QString &qName, const QXmlAttributes &attributes)
{
    if(namespaceURI != XmlFile::NAMESPACE_URI) return true; // XXX: is it correct behaviour ?

    switch(_state) {
    case START:
        if(qName == "world") {
            _item = _world;
            _state = WORLD;
        } else {
            _errorString = QObject::tr("The file is not a StepCoreXML file.");
            return false;
        }
        break;

    case WORLD:
        if(qName == "item") {
            Item* obj = _factory->newItem(attributes.value("class")); 
            if(obj == NULL) {
                _errorString = QObject::tr("Unknown item type \"%1\"").arg(attributes.value("class"));
                return false;
            }
            _world->addItem(obj);
            _item = obj;
            _state = ITEM;
            break;
        } else if(qName == "solver") {
            Solver* obj = _factory->newSolver(attributes.value("class")); 
            if(obj == NULL) {
                _errorString = QObject::tr("Unknown solver type \"%1\"").arg(attributes.value("class"));
                return false;
            }
            _world->setSolver(obj);
            _item = obj;
            _state = ITEM;
            break;
        }

    case ITEM:
        _propertyIdx = _item->metaObject()->indexOfProperty(qName.toAscii().constData());
        if(_propertyIdx == -1 || !_item->metaObject()->property(_propertyIdx).isStored(_item)) {
            _errorString = QObject::tr("Item \"%1\" has no property named \"%2\"")
                                .arg(QString(_item->metaObject()->className()).remove("StepCore::")).arg(qName);
            return false;
        }
        _text.clear();
        if(_state == WORLD) _state = WORLD_PROPERTY;
        else _state = ITEM_PROPERTY;
        break;

    case WORLD_PROPERTY:
    case ITEM_PROPERTY:
    default:
        _errorString = QObject::tr("Unexpected tag \"%1\"").arg(qName);
        return false;
    }

    return true;
}

bool XmlFileHandler::endElement(const QString &namespaceURI, const QString &localName,
                const QString &qName)
{
    if(namespaceURI != XmlFile::NAMESPACE_URI) return true; // XXX: is it correct behaviour ?

    switch(_state) {
    case WORLD_PROPERTY:
    case ITEM_PROPERTY:
        {
            QMetaProperty property = _item->metaObject()->property(_propertyIdx);
            STEPCORE_ASSERT_NOABORT(property.write(_item, _factory->stringToVariant(property.userType(), _text)));
        }
        if(_state == WORLD_PROPERTY) _state = WORLD;
        else _state = ITEM;
        break;

    case ITEM:
        _state = WORLD;
        break;

    case WORLD:
        _state = END;
        break;

    default:
        STEPCORE_ASSERT_NOABORT(false);
    }

    return true;
}

bool XmlFileHandler::characters(const QString &str)
{
    //if(_state == WORLD_PROPERTY || _state == ITEM_PROPERTY)
    _text += str;
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

bool XmlFile::load(World* world)
{
    XmlFileHandler handler(world, _factory);
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

