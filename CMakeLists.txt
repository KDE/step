project(step)

include(StepConfigureChecks.cmake)

set(STEPCORE_TESTS OFF CACHE BOOL "Build StepCore tests")
set(STEPCORE_WITH_GSL ${GSL_FOUND} CACHE BOOL "Enable GSL-powered features for StepCore")
set(STEPCORE_WITH_QT ON) # CACHE BOOL "Enable QT-powered features for StepCore")
set(STEP_WITH_QALCULATE ${QALCULATE_FOUND} CACHE BOOL "Enable unit conversion support using libqalculate")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${GMM_INCLUDE_DIR})
add_definitions(-DGETFEM_PARA_LEVEL=0)

add_subdirectory(stepcore)
add_subdirectory(step)

if(STEPCORE_TESTS)
  add_subdirectory(stepcore_tests)
endif(STEPCORE_TESTS)
