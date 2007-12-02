project(step)
enable_testing()

cmake_minimum_required(VERSION 2.4.5)
set(QT_MIN_VERSION "4.3.0") # we need at least version 4.3 of Qt
if (CMAKE_COMPILER_IS_GNUCXX)
	set ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} -Wno-long-long -std=iso9899:1990 -Wundef -Wcast-align -Werror-implicit-function-declaration -Wchar-subscripts -Wall -W -Wpointer-arith -Wwrite-strings -Wformat-security -Wmissing-format-attribute -fno-common")
	set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor -Wno-long-long -ansi -Wundef -Wcast-align -Wchar-subscripts -Wall -W -Wpointer-arith -Wformat-security -fno-exceptions -fno-check-new -fno-common")
	add_definitions (-D_BSD_SOURCE)
endif (CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )

find_package(GSL)
find_package(GMM)

if (GMM_INCLUDE_DIR)

set(QALCULATE_MIN_VERSION "0.9.5")
include(FindQalculate.cmake)

set(STEPCORE_TESTS OFF CACHE BOOL "Build StepCore tests")
set(STEPCORE_WITH_GSL ${GSL_FOUND} CACHE BOOL "Enable GSL-powered features for StepCore")
set(STEPCORE_WITH_QT ON) # CACHE BOOL "Enable QT-powered features for StepCore")
set(STEP_WITH_QALCULATE ${QALCULATE_FOUND} CACHE BOOL "Enable unit conversion support using libqalculate")


include_directories(${GMM_INCLUDE_DIR})
add_definitions(-DGETFEM_PARA_LEVEL=0)

include_directories(.)
add_subdirectory(stepcore)
add_subdirectory(step)

if(STEPCORE_TESTS)
add_subdirectory(stepcore_tests)
endif(STEPCORE_TESTS)

else (GMM_INCLUDE_DIR)
MESSAGE(STATUS "Step will not be built because you are missing GMM")
endif (GMM_INCLUDE_DIR)
