/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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

