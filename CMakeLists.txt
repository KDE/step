project(step)

cmake_minimum_required(VERSION 2.6.0)
set(KDE_MIN_VERSION "4.3")

find_package(KDE4 REQUIRED)

include(KDE4Defaults)

set(CMAKE_MODULE_PATH ${step_SOURCE_DIR}/cmake/modules )

find_package(Eigen2 2.0.3)
macro_optional_find_package(GSL)
macro_optional_find_package(Qalculate)

macro_log_feature(EIGEN2_FOUND "Eigen2" "Required to build step." "http://eigen.tuxfamily.org" TRUE "2.0.3" "")
macro_log_feature(QALCULATE_FOUND "Qalculate" "Unit conversion support in Step" "http://qalculate.sourceforge.net/" FALSE "0.9.5" "")
macro_log_feature(GSL_FOUND "GSL" "Provides a GSL-powered solver for Step" "http://www.gnu.org/software/gsl/" FALSE "1.8" "")

set(STEPCORE_WITH_GSL ${GSL_FOUND} CACHE BOOL "Enable GSL-powered features for StepCore")
set(STEPCORE_WITH_QT ON) # CACHE BOOL "Enable QT-powered features for StepCore")
set(STEP_WITH_QALCULATE ${QALCULATE_FOUND} CACHE BOOL "Enable unit conversion support using libqalculate")

if (EIGEN2_FOUND)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${EIGEN2_INCLUDE_DIR})

   add_subdirectory(doc) 
   add_subdirectory(stepcore)
   add_subdirectory(step)

   add_subdirectory(stepcore_tests)
endif (EIGEN2_FOUND)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
   macro_display_feature_log()
endif (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
