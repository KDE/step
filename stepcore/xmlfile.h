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

/** \file xmlfile.h
 *  \brief XmlFile class
 */

#ifndef STEPCORE_XMLFILE_H
#define STEPCORE_XMLFILE_H

#include "util.h"

#ifdef STEPCORE_WITH_QT

#include <QString>
class QIODevice;
class QTextStream;

namespace StepCore {

class World;
class Factory;

/** \ingroup xmlfile
 *  \brief Class for saving and loading World as XML file
 */
class XmlFile {
public:
    /** Constructs XmlFile
     *  \param device QIODevice to save or load file
     *  \param factory Factory to use when saving or loading file
     */
    XmlFile(QIODevice* device, const Factory* factory);

    /** Save world to XML file
     *  \param world World to save
     *  \return true on success, false on failure
     *          (with error message in errorString())
     */
    bool save(const World* world);

    /** Load world from XML file
     *  \param world World to which file should be loaded (should be empty)
     *  \return true on success, false on failure
     *          (with error message in errorString())
     */
    bool load(World* world);

    /** Get error message from last failed save() or load() */
    QString errorString() const;

protected:
    QIODevice*     _device;
    const Factory* _factory;

    QString _errorString;

    QString escapeText(const QString& str);
    void saveProperties(int indent, const QObject* obj, QTextStream& stream);
    void saveObject(int indent, const QString& tag, const QObject* obj, QTextStream& stream);

public:
    static const char* DOCKTYPE;
    static const char* NAMESPACE_URI;
    static const char* VERSION;

protected:
    static const int   INDENT;
};

}

#endif // STEPCORE_WITH_QT

#endif // STEPCORE_XMLFILE_H

