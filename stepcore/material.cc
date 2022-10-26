/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
    SPDX-FileCopyrightText: 2014 Inge Wallin <inge@lysator.liu.se>

    SPDX-License-Identifier: GPL-2.0-or-later
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
