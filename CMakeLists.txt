project(step)

macro_optional_find_package(GSL)
macro_optional_find_package(Qalculate)

macro_log_feature(QALCULATE_FOUND "Qalculate" "A multi-purpose desktop calculator" "http://qalculate.sourceforge.net/" FALSE "0.9.5" "Enable unit conversion support in Step")
macro_log_feature(GSL_FOUND "GSL" "The GNU Scientific Library, a numerical library for C and C++" "http://www.gnu.org/software/gsl/" FALSE "1.8" "Enables GSL-powered features in Step")

set(STEPCORE_TESTS OFF CACHE BOOL "Build StepCore tests")
set(STEPCORE_WITH_GSL ${GSL_FOUND} CACHE BOOL "Enable GSL-powered features for StepCore")
set(STEPCORE_WITH_QT ON) # CACHE BOOL "Enable QT-powered features for StepCore")
set(STEP_WITH_QALCULATE ${QALCULATE_FOUND} CACHE BOOL "Enable unit conversion support using libqalculate")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${EIGEN2_INCLUDE_DIR})

# -fexceptions is required because (ironically) of the use of new operator
# definitions from Eigen that don't throw exceptions (but that use
# try...catch blocks to achieve this)
add_definitions(-fexceptions)

add_subdirectory(stepcore)
add_subdirectory(step)

if(STEPCORE_TESTS)
  add_subdirectory(stepcore_tests)
endif(STEPCORE_TESTS)
