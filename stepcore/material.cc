/* This file is part of StepCore library.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
   Copyright (C) 2014 Inge Wallin        <inge@lysator.liu.se>

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

#include "material.h"


namespace StepCore
{

//STEPCORE_META_OBJECT(Body, QT_TRANSLATE_NOOP("ObjectClass", "Body"), QT_TR_NOOP("Body"),
//		     MetaObject::ABSTRACT,,)


Material& Material::operator=(const Material& mtrl)
{
    _name    = mtrl._name;
    _color   = mtrl._color;
    _density = mtrl._density;

    return *this;
}


Material GenericMaterial(QStringLiteral("Bloontonium"));


} // namespace StepCore
