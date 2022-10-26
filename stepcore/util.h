/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/** \file util.h
 *  \brief Internal file
 */

#ifndef STEPCORE_UTIL_H
#define STEPCORE_UTIL_H

namespace StepCore {

template<typename T> inline T square(T v) { return v*v; }

/*
template<class _Class, class _BaseIterator>
class ClassFilterIterator
{
public:
    ClassFilterIterator(const _BaseIterator& it): _it(it);
protected:
    _BaseIterator _it;
};
*/

} // namespace StepCore

#ifdef __GNUC__
#define STEPCORE_UNUSED __attribute__((unused))
#else
#define STEPCORE_UNUSED
#endif

#define STEPCORE_STRINGIFY(x) _STEPCORE_STRINGIFY(x)
#define _STEPCORE_STRINGIFY(x) #x

#ifdef NDEBUG
#define STEPCORE_ASSERT_NOABORT(expr)
#else // NDEBUG

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define STEPCORE_ASSERT_NOABORT(expr) \
    if( ! (expr) ) \
        StepCore::_step_assert_noabort_helper<int> \
            ( STEPCORE_STRINGIFY(expr), __LINE__, \
              __FILE__, __PRETTY_FUNCTION__ )

namespace StepCore {
template<typename unused>
void _step_assert_noabort_helper( const char *expr, int line,
                                   const char *file, const char *function )
{
#ifdef STEPCORE_WITH_QT
    qCritical("*** StepCore: failed assertion on line %d of file %s\n"
                      "*** asserted expression: %s\n"
                      "*** in function: %s\n",
                      line, file, expr, function);
#else
    fprintf(stderr, "*** StepCore: failed assertion on line %d of file %s\n"
                      "*** asserted expression: %s\n"
                      "*** in function: %s\n",
                      line, file, expr, function);
#endif
}
} // namespace StepCore

#endif // ! NDEBUG

#endif

