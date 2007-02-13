project(step)
enable_testing()

include(FindGSL.cmake)
set(STEPCORE_TESTS OFF CACHE BOOL "Build StepCore tests")
set(STEPCORE_WITH_GSL ${GSL_FOUND} CACHE BOOL "Enable GSL-powered features for StepCore")
set(STEPCORE_WITH_QT ON) # CACHE BOOL "Enable QT-powered features for StepCore")

include_directories(.)
add_subdirectory(stepcore)
add_subdirectory(step)

if(STEPCORE_TESTS)
add_subdirectory(stepcore_tests)
endif(STEPCORE_TESTS)

