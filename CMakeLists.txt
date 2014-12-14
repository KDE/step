project(step)

cmake_minimum_required(VERSION 2.6.0)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

find_package(Qt5 5.2 REQUIRED NO_MODULE COMPONENTS Qml Quick Xml Svg OpenGL PrintSupport Test)
find_package(KF5 REQUIRED COMPONENTS KHtml Config KDELibs4Support NewStuff Plotting)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(FeatureSummary)

find_package(Eigen2 2.0.3 REQUIRED)
find_package(GSL)
find_package(Qalculate)

set_package_properties(EIGEN2 PROPERTIES
# 		       REQUIRED_VERSION "2.0.3"
		       DESCRIPTION "Eigen2"
                       URL "http://eigen.tuxfamily.org"
                       TYPE OPTIONAL
                       PURPOSE "Required to build step")

set_package_properties(QALCULATE PROPERTIES
# 		       REQUIRED_VERSION "0.9.5"
		       DESCRIPTION "Qalculate"
                       URL "http://qalculate.sourceforge.net/"
                       TYPE OPTIONAL
                       PURPOSE "Unit conversion support in Step")

set_package_properties(GSL PROPERTIES
# 		       REQUIRED_VERSION "1.8"
		       DESCRIPTION "GSL"
                       URL "http://eigen.tuxfamily.org"
                       TYPE OPTIONAL
                       PURPOSE "Provides a GSL-powered solver for Step")

set(STEPCORE_WITH_GSL ${GSL_FOUND} CACHE BOOL "Enable GSL-powered features for StepCore")
set(STEPCORE_WITH_QT ON) # CACHE BOOL "Enable QT-powered features for StepCore")
set(STEP_WITH_QALCULATE ${QALCULATE_FOUND} CACHE BOOL "Enable unit conversion support using libqalculate")

# This setting is forced by Eigen2 creating many many warnings without it.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")


if (EIGEN2_FOUND)
   include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${EIGEN2_INCLUDE_DIR})

   find_package(KF5DocTools REQUIRED)
   
   add_subdirectory(doc)
   add_subdirectory(stepcore)
   add_subdirectory(step)

   add_subdirectory(stepcore_tests)
endif (EIGEN2_FOUND)

install(FILES step.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
   # FIXME: What's the equivalent of this in KF5?
   #macro_display_feature_log()
endif (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

