# - Try to find the freetype library
#  GSL_FOUND - system has libusb
#  GSL_INCLUDE_DIR - the libusb include directory
#  GSL_LIBRARIES - Link these to use libusb

# Copyright (c) 2008, Allen Winter <winter@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (GSL_INCLUDE_DIR AND GSL_LIBRARIES AND GSL_CBLAS_LIBRARIES)

  # Already in cache, be silent
  set(GSL_FIND_QUIETLY TRUE)

else (GSL_INCLUDE_DIR AND GSL_LIBRARIES AND GSL_CBLAS_LIBRARIES)
  if(NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    include(FindPkgConfig)
    pkg_check_modules(_pc_GSL gsl)
  else(NOT WIN32)
    set(_pc_GSL_FOUND TRUE)
  endif(NOT WIN32)

  if(_pc_GSL_FOUND)
    find_library(GSL_LIBRARIES
      NAMES gsl
      HINTS ${_pc_GSL_LIBRARY_DIRS} ${CMAKE_LIBRARY_PATH}
    )

    find_library(GSL_CBLAS_LIBRARIES
      NAMES gslcblas
      HINTS ${_pc_GSL_LIBRARY_DIRS} ${CMAKE_LIBRARY_PATH}
    )

    find_path(GSL_INCLUDE_DIR
      NAMES gsl/gsl_cdf.h gsl/gsl_randist.h
      HINTS ${_pc_GSL_INCLUDE_DIRS}
      PATH_SUFFIXES gsl
    )

  endif(_pc_GSL_FOUND)

  mark_as_advanced(GSL_INCLUDE_DIR GSL_LIBRARIES GSL_CBLAS_LIBRARIES)

endif (GSL_INCLUDE_DIR AND GSL_LIBRARIES AND GSL_CBLAS_LIBRARIES)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GSL DEFAULT_MSG
                                  GSL_LIBRARIES GSL_CBLAS_LIBRARIES GSL_INCLUDE_DIR)
