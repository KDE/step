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

#ifndef STEPCORE_VECTOR_H
#define STEPCORE_VECTOR_H

#include <Eigen/Core>
#include <QMetaType>

namespace StepCore
{

/** Two-dimensional vector with double components */
typedef Eigen::Vector2d Vector2d;
/** Three-dimensional vector with double components*/
typedef Eigen::Vector3d Vector3d;

/** Two-dimensional vector with integer components */
typedef Eigen::Vector2i Vector2i;
/** Three-dimensional vector with integer components */
typedef Eigen::Vector3i Vector3i;

typedef Eigen::VectorXd VectorXd;

} // namespace StepCore

// XXX: move it to types.h
Q_DECLARE_METATYPE(StepCore::Vector2d)
Q_DECLARE_METATYPE(StepCore::Vector3d)
Q_DECLARE_METATYPE(StepCore::Vector2i)
Q_DECLARE_METATYPE(StepCore::Vector3i)

#endif

