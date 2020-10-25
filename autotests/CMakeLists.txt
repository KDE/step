# Copyright 2014-2015  Andreas Cord-Landwehr <cordlandwehr@kde.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

include(ECMMarkAsTest)

find_package(Qt5Test ${QT_MIN_VERSION} CONFIG QUIET)

if(STEPCORE_WITH_QT)
    add_definitions(-DSTEPCORE_WITH_QT)
endif(STEPCORE_WITH_QT)

if(STEPCORE_WITH_GSL)
    add_definitions(-DSTEPCORE_WITH_GSL)
endif(STEPCORE_WITH_GSL)

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

macro(STEPCORE_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cc)
      add_test(NAME stepcore-${_testname} COMMAND ${_testname})
      target_link_libraries(${_testname} stepcore Qt5::Test)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

stepcore_unit_tests(
    test_contacts
    test_metaobject
    test_forces
#     test_worldcopy #FIXME must be reenabled after sorting out derivation issues
)
