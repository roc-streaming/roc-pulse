if(EXISTS ${PULSEAUDIO_DIR}/.tarball-version)
  file(READ ${PULSEAUDIO_DIR}/.tarball-version SOURCE_PULSEAUDIO_VERSION)
  string(REGEX REPLACE "\n$" "" SOURCE_PULSEAUDIO_VERSION "${SOURCE_PULSEAUDIO_VERSION}")
endif()

if(SOURCE_PULSEAUDIO_VERSION STREQUAL "")
  message(STATUS "Detected SOURCE_PULSEAUDIO_VERSION - None")
else()
  message(STATUS "Detected SOURCE_PULSEAUDIO_VERSION - ${SOURCE_PULSEAUDIO_VERSION}")
endif()
