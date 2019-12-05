cmake_minimum_required(VERSION 3.5)

# KDE Application Version, managed by release script
set(KDE_APPLICATIONS_VERSION_MAJOR "19")
set(KDE_APPLICATIONS_VERSION_MINOR "12")
set(KDE_APPLICATIONS_VERSION_MICRO "0")
set(KDE_APPLICATIONS_VERSION "${KDE_APPLICATIONS_VERSION_MAJOR}.${KDE_APPLICATIONS_VERSION_MINOR}.${KDE_APPLICATIONS_VERSION_MICRO}")

project(step VERSION ${KDE_APPLICATIONS_VERSION})

find_package(ECM 1.7.0 REQUIRED NO_MODULE)
find_package(KF5DocTools)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

include(KDEInstallDirs)
include(ECMAddTests)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(ECMOptionalAddSubdirectory)
include(ECMSetupVersion)
include(FeatureSummary)
include(ECMPoQmTools)

find_package(Qt5 5.2 REQUIRED NO_MODULE COMPONENTS
    Xml
    Svg
    OpenGL
    Test
)

find_package(KF5 REQUIRED COMPONENTS
    Crash
    Config
    I18n
    IconThemes
    KIO
    NewStuff
    Plotting
    TextWidgets
)
find_package(Eigen3 3.2.2 REQUIRED)
find_package(GSL)
find_package(Qalculate)

set_package_properties(EIGEN3 PROPERTIES
                       DESCRIPTION "Eigen3"
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

# These settings are forced by Eigen2 creating many many warnings without them.
# FIXME: How can we make Eigen behave and not need them?
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${EIGEN3_INCLUDE_DIR})

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x060000)
if(KF5DocTools_FOUND)
    ecm_optional_add_subdirectory(doc)
endif()
add_subdirectory(stepcore)
add_subdirectory(step)
add_subdirectory(autotests)

install(FILES org.kde.step.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

find_package(SharedMimeInfo REQUIRED)
install(FILES org.kde.step.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
