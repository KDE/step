project(step)
enable_testing()

include(FindGSL.cmake)
set(STEPCORE_WITH_GSL ${GSL_FOUND} CACHE BOOL "Enable GSL-powered features for StepCore")
set(STEPCORE_WITH_QT ON) # CACHE BOOL "Enable QT-powered features for StepCore")

include_directories(.)
subdirs(stepcore step)


