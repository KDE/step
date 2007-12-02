if(GMM_INCLUDE_DIR)

  # in cache already
  set(GMM_FOUND TRUE)

else(GMM_INCLUDE_DIR)

find_path(GMM_INCLUDE_DIR gmm/gmm.h PATHS ${INCLUDE_INSTALL_DIR})
if(GMM_INCLUDE_DIR)
    if(NOT GMM_FIND_QUIETLY)
      message(STATUS "Found GMM++: ${GMM_INCLUDE_DIR}")
    endif(NOT GMM_FIND_QUIETLY)
else(GMM_INCLUDE_DIR)
    if(GMM_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find GMM++")
    endif(GMM_FIND_REQUIRED)
endif(GMM_INCLUDE_DIR)

mark_as_advanced(GMM_INCLUDE_DIR)

endif(GMM_INCLUDE_DIR)
