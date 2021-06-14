cmake_minimum_required(VERSION 3.0.0)

project(roc_pulse C)

set(HOST "" CACHE STRING "target host triple, e.g. aarch64-linux-gnu")

set(ROC_INCLUDE_DIR "" CACHE STRING "roc toolkit include directory")
set(ROC_LIBRARY_DIR "" CACHE STRING "roc toolkit library directory")

set(PULSEAUDIO_SOURCE_DIR "" CACHE STRING "directory with pulseaudio source code")
set(PULSEAUDIO_BUILD_DIR "" CACHE STRING "directory with pulseaudio build results")

if(NOT HOST STREQUAL "")
  set(CMAKE_CROSSCOMPILING ON)
  set(CMAKE_SYSTEM_NAME Linux)

  find_program(C_COMPILER ${HOST}-gcc)
  if(C_COMPILER STREQUAL "C_COMPILER-NOTFOUND")
    find_program(C_COMPILER ${HOST}-clang)
  endif()

  set(CMAKE_C_COMPILER "${C_COMPILER}")
  message(STATUS "Using CMAKE_C_COMPILER - ${CMAKE_C_COMPILER}")
endif()

if(NOT CMAKE_CROSSCOMPILING)
  include("cmake/detect_pulseaudio.cmake")
endif()

if(PULSEAUDIO_SOURCE_DIR STREQUAL "")
  include("cmake/build_pulseaudio.cmake")
else()
  if(PULSEAUDIO_BUILD_DIR STREQUAL "")
    set(PULSEAUDIO_BUILD_DIR "${PULSEAUDIO_SOURCE_DIR}")
  endif()

  get_filename_component(PULSEAUDIO_SOURCE_DIR "${PULSEAUDIO_SOURCE_DIR}" ABSOLUTE)
  get_filename_component(PULSEAUDIO_BUILD_DIR "${PULSEAUDIO_BUILD_DIR}" ABSOLUTE)

  message(STATUS "Using PULSEAUDIO_SOURCE_DIR - ${PULSEAUDIO_SOURCE_DIR}")
  message(STATUS "Using PULSEAUDIO_BUILD_DIR - ${PULSEAUDIO_BUILD_DIR}")

  include_directories(SYSTEM
    "${PULSEAUDIO_SOURCE_DIR}/src"
    "${PULSEAUDIO_BUILD_DIR}"
    )
endif()

if(NOT ROC_INCLUDE_DIR STREQUAL "")
  get_filename_component(ROC_INCLUDE_DIR "${ROC_INCLUDE_DIR}" ABSOLUTE)
  message(STATUS "Using ROC_INCLUDE_DIR - ${ROC_INCLUDE_DIR}")

  include_directories(SYSTEM "${ROC_INCLUDE_DIR}")
endif()

if(NOT ROC_LIBRARY_DIR STREQUAL "")
  get_filename_component(ROC_LIBRARY_DIR "${ROC_LIBRARY_DIR}" ABSOLUTE)
  message(STATUS "Using ROC_LIBRARY_DIR - ${ROC_LIBRARY_DIR}")

  link_directories("${ROC_LIBRARY_DIR}")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} -fPIC")

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-variable")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_C_FLAGS
    "${CMAKE_C_FLAGS} -Weverything -Wno-unused-variable")
endif()

add_library(rocpulse_helpers OBJECT
  "src/rocpulse_helpers.c"
  )

if(PULSEAUDIO_SOURCE_DIR STREQUAL "")
  add_dependencies(rocpulse_helpers
    pulseaudio
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
