project(step)

include(StepConfigureChecks.cmake)

if (GMM_INCLUDE_DIR)
   set(STEPCORE_TESTS OFF CACHE BOOL "Build StepCore tests")
   set(STEPCORE_WITH_GSL ${GSL_FOUND} CACHE BOOL "Enable GSL-powered features for StepCore")
   set(STEPCORE_WITH_QT ON) # CACHE BOOL "Enable QT-powered features for StepCore")
   set(STEP_WITH_QALCULATE ${QALCULATE_FOUND} CACHE BOOL "Enable unit conversion support using libqalculate")

   if(NOT GSL_FOUND)
       set(STEPCORE_WITH_GSL OFF CACHE BOOL "Enable GSL-powered features for StepCore" FORCE)
   endif(NOT GSL_FOUND)

   include_directories(${GMM_INCLUDE_DIR})
   add_definitions(-DGETFEM_PARA_LEVEL=0)

   include_directories(.)
   add_subdirectory(stepcore)
   add_subdirectory(step)

   if(STEPCORE_TESTS)
      add_subdirectory(stepcore_tests)
   endif(STEPCORE_TESTS)

else (GMM_INCLUDE_DIR)
   message(STATUS "Step will not be built because you are missing GMM.")
   message(STATUS "Download the latest stable version of GMM from:")
   message(STATUS "http://home.gna.org/getfem/download.html")
endif (GMM_INCLUDE_DIR)
