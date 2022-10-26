# SPDX-FileCopyrightText: 2014-2015 Andreas Cord-Landwehr <cordlandwehr@kde.org>
#
# SPDX-License-Identifier: BSD-2-Clause.

include(ECMMarkAsTest)

find_package(Qt${QT_MAJOR_VERSION}Test ${QT_MIN_VERSION} CONFIG QUIET)

if(NOT Qt${QT_MAJOR_VERSION}Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

macro(STEPCORE_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cc)
      add_test(NAME stepcore-${_testname} COMMAND ${_testname})
      target_include_directories(${_testname} PUBLIC ${CMAKE_SOURCE_DIR})
      target_link_libraries(${_testname} stepcore Qt::Test)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

stepcore_unit_tests(
    test_contacts
    test_metaobject
    test_forces
#     test_worldcopy #FIXME must be reenabled after sorting out derivation issues
)
