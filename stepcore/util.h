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

/** \file util.h
 *  \brief Internal file
 */

#ifndef STEPCORE_UTIL_H
#define STEPCORE_UTIL_H

#ifdef STEPCORE_WITH_QT

#include <QObject>
#include <QMetaType>
namespace StepCore { class StepCoreObject: public QObject {}; }
#define STEPCORE_PRINT_ERR(...) qCritical(__VA_ARGS__)

#else // STEPCORE_WITH_QT

#if defined(Q_OBJECT) || defined(Q_PROPERTY) || defined(Q_DECLARE_METATYPE)
#error "Trying to build StepCore without QT, but QT headers are included !"
#endif

#define Q_OBJECT
#define Q_PROPERTY(...)
#define Q_DECLARE_METATYPE(...)

namespace StepCore { class StepCoreObject: {}; }

#include <stdio.h>
#define STEPCORE_PRINT_ERR(...) fprintf(stderr, __VA_ARGS__)

#endif // ! STEPCORE_WITH_QT

#ifdef __GNUC__
#define STEPCORE_UNUSED __attribute__((unused))
#else
#define STEPCORE_UNUSED
#endif

namespace StepCore {

#ifdef NDEBUG
#define STEPCORE_ASSERT_NOABORT(expr)
#else // NDEBUG

template<typename unused>
void _step_assert_noabort_helper( const char *expr, int line,
                                   const char *file, const char *function )
{
    STEPCORE_PRINT_ERR("*** StepCore: failed assertion on line %d of file %s\n"
                      "*** asserted expression: %s\n"
                      "*** in function: %s\n",
                      line, file, expr, function);
}

#define STEPCORE_ASSERT_NOABORT(expr) \
    if( ! (expr) ) \
        StepCore::_step_assert_noabort_helper<int> \
            ( __STRING(expr), __LINE__, \
              __FILE__, __PRETTY_FUNCTION__ )

#endif // ! NDEBUG

} // namespace StepCore

#endif

