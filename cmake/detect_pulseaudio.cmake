execute_process(
  COMMAND sh -c "pulseaudio --version | sed -e 's,^[^0-9]*,,'"
  ERROR_QUIET
  OUTPUT_VARIABLE SYSTEM_PULSEAUDIO_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

if(SYSTEM_PULSEAUDIO_VERSION STREQUAL "")
  message(STATUS "Detected SYSTEM_PULSEAUDIO_VERSION - None")
else()
  message(STATUS "Detected SYSTEM_PULSEAUDIO_VERSION - ${SYSTEM_PULSEAUDIO_VERSION}")
endif()

execute_process(
  COMMAND sh -c "pulseaudio --dump-conf | grep dl-search-path | sed -e 's,^[^=]*=\\s*,,'"
  ERROR_QUIET
  OUTPUT_VARIABLE SYSTEM_PULSEAUDIO_MODULE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

if(SYSTEM_PULSEAUDIO_MODULE_DIR STREQUAL "")
  set(LIB_DIRS
    "lib"
    )

  if(CMAKE_HOST_SYSTEM_VERSION MATCHES "amd64|x86_64")
    list(APPEND LIB_DIRS
      "lib64"
      "lib/x86_64-linux-gnu"
      )
  endif()

  foreach(PREFIX IN ITEMS /usr/local /usr)
    foreach(LIBDIR IN LISTS LIB_DIRS)
      set(MODULE_DIR "${PREFIX}/${LIBDIR}/pulse-${SYSTEM_PULSEAUDIO_VERSION}/modules")
      if(EXISTS "${MODULE_DIR}" AND SYSTEM_PULSEAUDIO_MODULE_DIR STREQUAL "")
        set(SYSTEM_PULSEAUDIO_MODULE_DIR "${MODULE_DIR}")
      endif()
    endforeach()
  endforeach()
endif()

if(SYSTEM_PULSEAUDIO_MODULE_DIR STREQUAL "")
  message(STATUS "Detected SYSTEM_PULSEAUDIO_MODULE_DIR - None")
else()
  message(STATUS "Detected SYSTEM_PULSEAUDIO_MODULE_DIR - ${SYSTEM_PULSEAUDIO_MODULE_DIR}")
endif()
