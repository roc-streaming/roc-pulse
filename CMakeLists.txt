cmake_minimum_required(VERSION 3.0.0)

cmake_policy(SET CMP0135 NEW)

project(roc_pulse C CXX)

set(HOST "" CACHE STRING "target host triplet, e.g. aarch64-linux-gnu")

include("cmake/setup_toolchain.cmake")

option(DOWNLOAD_PULSEAUDIO "automatically download pulseaudio" ON)
option(DOWNLOAD_LIBTOOL "automatically download libtool (ltdl)" ON)
option(DOWNLOAD_ROC "automatically download and build roc-toolkit" ON)

set(DEFAULT_PULSEAUDIO_VERSION "")

if(NOT CMAKE_CROSSCOMPILING)
  include("cmake/detect_pulseaudio.cmake")

  if(NOT SYSTEM_PULSEAUDIO_VERSION STREQUAL "")
    set(DEFAULT_PULSEAUDIO_VERSION "${SYSTEM_PULSEAUDIO_VERSION}")
  endif()
endif()

set(PULSEAUDIO_VERSION "${DEFAULT_PULSEAUDIO_VERSION}" CACHE STRING "pulseaudio version")
set(PULSEAUDIO_DIR "" CACHE STRING "directory with pulseaudio source code")

set(LIBTOOL_VERSION "2.4.6" CACHE STRING "libtool version")
set(LIBTOOL_DIR "" CACHE STRING "directory with libtool source code")

set(ROC_INCLUDE_DIR "" CACHE STRING "roc toolkit include directory")
set(ROC_LIB_DIR "" CACHE STRING "roc toolkit library directory")

if(PULSEAUDIO_VERSION STREQUAL "")
  message(FATAL_ERROR "can't auto-detect PULSEAUDIO_VERSION, define it manually")
else()
  message(STATUS "Using PULSEAUDIO_VERSION - ${PULSEAUDIO_VERSION}")
endif()

if(DOWNLOAD_PULSEAUDIO)
  include("cmake/download_pulseaudio.cmake")
else()
  if(PULSEAUDIO_DIR STREQUAL "")
    message(FATAL_ERROR "if DOWNLOAD_PULSEAUDIO is OFF, PULSEAUDIO_DIR should be defined")
  endif()

  get_filename_component(PULSEAUDIO_DIR "${PULSEAUDIO_DIR}" ABSOLUTE)

  message(STATUS "Using PULSEAUDIO_DIR - ${PULSEAUDIO_DIR}")

  include_directories(SYSTEM
    "${PULSEAUDIO_DIR}/src"
    )
endif()

if(DOWNLOAD_LIBTOOL)
  if(LIBTOOL_VERSION STREQUAL "")
    message(FATAL_ERROR "if DOWNLOAD_LIBTOOL is ON, LIBTOOL_VERSION should be defined")
  else()
    message(STATUS "Using LIBTOOL_VERSION - ${LIBTOOL_VERSION}")
  endif()

  include("cmake/download_libtool.cmake")
elseif(NOT LIBTOOL_DIR STREQUAL "")
  get_filename_component(LIBTOOL_DIR "${LIBTOOL_DIR}" ABSOLUTE)

  message(STATUS "Using LIBTOOL_DIR - ${LIBTOOL_DIR}")

  include_directories(SYSTEM
    "${LIBTOOL_DIR}/libltdl"
    )
endif()

if(DOWNLOAD_ROC)
  include("cmake/download_roc.cmake")
else()
  if(NOT ROC_INCLUDE_DIR STREQUAL "")
    get_filename_component(ROC_INCLUDE_DIR "${ROC_INCLUDE_DIR}" ABSOLUTE)
    message(STATUS "Using ROC_INCLUDE_DIR - ${ROC_INCLUDE_DIR}")

    include_directories(SYSTEM "${ROC_INCLUDE_DIR}")
  endif()

  if(NOT ROC_LIB_DIR STREQUAL "")
    get_filename_component(ROC_LIB_DIR "${ROC_LIB_DIR}" ABSOLUTE)
    message(STATUS "Using ROC_LIB_DIR - ${ROC_LIB_DIR}")

    link_directories("${ROC_LIB_DIR}")
  endif()

  link_libraries("roc")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

if(NOT CMAKE_C_FLAGS MATCHES "-fPIC")
  set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -fPIC")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-variable")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -Weverything -Wno-unused-variable")
endif()

add_definitions(
  -DROC_PULSEAUDIO_VERSION=${PULSEAUDIO_VERSION}
  )

include_directories("src")

add_library(rocpulse_helpers OBJECT
  "src/rocpulse_helpers.c"
  )

if(DOWNLOAD_PULSEAUDIO)
  add_dependencies(rocpulse_helpers
    pulseaudio
    )
endif()

if(DOWNLOAD_LIBTOOL)
  add_dependencies(rocpulse_helpers
    libtool
    )
endif()

if(DOWNLOAD_ROC)
  add_dependencies(rocpulse_helpers
    roc
    )
endif()

add_library(module-roc-sink SHARED
  $<TARGET_OBJECTS:rocpulse_helpers>
  "src/module-roc-sink.c"
  )

add_library(module-roc-sink-input SHARED
  $<TARGET_OBJECTS:rocpulse_helpers>
  "src/module-roc-sink-input.c"
  )

set(ALL_MODULES
  module-roc-sink
  module-roc-sink-input
  )

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY ON)

if(CMAKE_CROSSCOMPILING OR SYSTEM_PULSEAUDIO_MODULE_DIR STREQUAL "")
  set(MODULE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib/pulse-${PULSEAUDIO_VERION}/modules")
else()
  set(MODULE_INSTALL_DIR "${SYSTEM_PULSEAUDIO_MODULE_DIR}")
endif()

foreach(MODULE IN LISTS ALL_MODULES)
  install(
    FILES $<TARGET_FILE:${MODULE}>
    DESTINATION "${MODULE_INSTALL_DIR}"
    )
endforeach()

add_custom_target(compile_commands ALL
  DEPENDS ${ALL_MODULES}
  COMMENT "Copying compile_commands.json to project root"
  COMMAND "${CMAKE_COMMAND}" -E copy
    "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    "${PROJECT_SOURCE_DIR}/compile_commands.json"
  )
