cmake_minimum_required(VERSION 3.16)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "25")
set(RELEASE_SERVICE_VERSION_MINOR "11")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(step VERSION ${RELEASE_SERVICE_VERSION})

set(KDE_COMPILERSETTINGS_LEVEL "5.84")
set(QT_MIN_VERSION "6.6.2")
set(KF_MIN_VERSION "6.7.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )
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

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS
    Xml
    Svg
    OpenGLWidgets
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Crash
    Config
    I18n
    IconThemes
    KIO
    NewStuff
    Plotting
    TextWidgets
    XmlGui
)
find_package(KF6DocTools ${KF_MIN_VERSION})
find_package(Eigen3 3.2.2 REQUIRED)
find_package(GSL)
find_package(Qalculate)

set_package_properties(EIGEN3 PROPERTIES
                       DESCRIPTION "Eigen3"
                       URL "https://eigen.tuxfamily.org"
                       TYPE OPTIONAL
                       PURPOSE "Required to build step")

set_package_properties(QALCULATE PROPERTIES
# 		       REQUIRED_VERSION "0.9.5"
                       DESCRIPTION "Qalculate"
                       URL "https://qalculate.github.io/"
                       TYPE OPTIONAL
                       PURPOSE "Unit conversion support in Step")

set_package_properties(GSL PROPERTIES
# 		       REQUIRED_VERSION "1.8"
                       DESCRIPTION "GSL"
                       URL "https://www.gnu.org/software/gsl/"
                       TYPE OPTIONAL
                       PURPOSE "Provides a GSL-powered solver for Step")

set(STEPCORE_WITH_GSL ${GSL_FOUND} CACHE BOOL "Enable GSL-powered features for StepCore")
set(STEPCORE_WITH_QT ON) # CACHE BOOL "Enable QT-powered features for StepCore")
set(STEP_WITH_QALCULATE ${QALCULATE_FOUND} CACHE BOOL "Enable unit conversion support using libqalculate")

# Compiler specific options
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # Clang specific options
    include(${CMAKE_CURRENT_SOURCE_DIR}/Clang.cmake)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # GCC specific options
    include(${CMAKE_CURRENT_SOURCE_DIR}/GCC.cmake)
else()
    # Some other compiler
endif()

if(KF6DocTools_FOUND)
    ecm_optional_add_subdirectory(doc)
endif()
add_subdirectory(stepcore)
add_subdirectory(step)
add_subdirectory(autotests)

ecm_install_po_files_as_qm(poqm)
ki18n_install(po)
if (KF6DocTools_FOUND)
    kdoctools_install(po)
endif()
install(FILES org.kde.step.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

find_package(SharedMimeInfo REQUIRED)
install(FILES org.kde.step.xml DESTINATION ${KDE_INSTALL_MIMEDIR})
update_xdg_mimetypes(${KDE_INSTALL_MIMEDIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
