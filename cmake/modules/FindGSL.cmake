# - Try to find the freetype library
#  GSL_FOUND - system has libusb
#  GSL_INCLUDE_DIR - the libusb include directory
#  GSL_LIBRARIES - Link these to use libusb

# Copyright (c) 2008, Allen Winter <winter@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (GSL_INCLUDE_DIR AND GSL_LIBRARIES)

  # Already in cache, be silent
  set(GSL_FIND_QUIETLY TRUE)

else (GSL_INCLUDE_DIR AND GSL_LIBRARIES)
  IF (NOT WIN32)
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    INCLUDE(UsePkgConfig)

    PKGCONFIG(gsl _GSLIncDir _GSLLinkDir _GSLLinkFlags _GSLCflags)
  ENDIF(NOT WIN32)

  find_path(GSL_INCLUDE_DIR
            NAMES gsl/gsl_cdf.h gsl/gsl_randist.h
            PATHS ${_GSLIncDir})
  set(GSL_LIBRARIES ${_GSLLinkFlags})

  mark_as_advanced(GSL_INCLUDE_DIR GSL_LIBRARIES)

endif (GSL_INCLUDE_DIR AND GSL_LIBRARIES)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GSL DEFAULT_MSG
                                  GSL_LIBRARIES GSL_INCLUDE_DIR)
