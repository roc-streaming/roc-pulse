if((CMAKE_MAJOR_VERSION EQUAL 3) AND (CMAKE_MINOR_VERSION LESS 5))
  cmake_minimum_required(VERSION 3.0)
else()
  cmake_minimum_required(VERSION 3.5)
endif()

if((CMAKE_MAJOR_VERSION GREATER 3) OR
    ((CMAKE_MAJOR_VERSION EQUAL 3) AND NOT (CMAKE_MINOR_VERSION LESS 24)))
  cmake_policy(SET CMP0135 NEW)
endif()

if("$ENV{CI}")
  set(USE_LOGFILES OFF)
else()
  set(USE_LOGFILES ON)
endif()

project(roc_pulse C CXX)

include("cmake/define_option.cmake")

define_option(TOOLCHAIN_PREFIX "" STRING "target host triplet, e.g. aarch64-linux-gnu")
include("cmake/setup_toolchain.cmake")

define_option(DOWNLOAD_PULSEAUDIO ON BOOL "automatically download pulseaudio")
define_option(DOWNLOAD_LIBTOOL ON BOOL "automatically download libtool (ltdl)")
define_option(DOWNLOAD_ROC ON BOOL "automatically download and build roc-toolkit")

define_option(PULSEAUDIO_VERSION "" STRING "pulseaudio version")
define_option(PULSEAUDIO_DIR "" STRING "directory with pulseaudio source code")
define_option(PULSEAUDIO_MODULE_DIR "" STRING "directory where to install pulseaudio modules")

define_option(LIBTOOL_VERSION "2.4.6" STRING "libtool version")
define_option(LIBTOOL_DIR "" STRING "directory with libtool source code")

define_option(ROC_REVISION "" STRING "roc toolkit revision to download")
define_option(ROC_INCLUDE_DIR "" STRING "roc toolkit include directory")
define_option(ROC_LIB_DIR "" STRING "roc toolkit library directory")

if(NOT PULSEAUDIO_VERSION)
  if(NOT PULSEAUDIO_DIR)
    if(NOT CMAKE_CROSSCOMPILING)
      include("cmake/detect_system_pulseaudio_version.cmake")
      if(SYSTEM_PULSEAUDIO_VERSION)
        set(PULSEAUDIO_VERSION "${SYSTEM_PULSEAUDIO_VERSION}")
      endif()
    endif()
  else()
    include("cmake/detect_source_pulseaudio_version.cmake")
    if(SOURCE_PULSEAUDIO_VERSION)
      set(PULSEAUDIO_VERSION "${SOURCE_PULSEAUDIO_VERSION}")
    endif()
  endif()
endif()

if(NOT PULSEAUDIO_VERSION)
  message(FATAL_ERROR "Can't auto-detect PULSEAUDIO_VERSION, define it manually")
else()
  message(STATUS "Using PULSEAUDIO_VERSION - ${PULSEAUDIO_VERSION}")
endif()

string(REPLACE "." ";" PULSEAUDIO_VERSION_PARTS ${PULSEAUDIO_VERSION})
list(GET PULSEAUDIO_VERSION_PARTS 0 PULSEAUDIO_VERSION_MAJOR)
list(GET PULSEAUDIO_VERSION_PARTS 1 PULSEAUDIO_VERSION_MINOR)

if(NOT PULSEAUDIO_MODULE_DIR)
  if(CMAKE_CROSSCOMPILING OR NOT SYSTEM_PULSEAUDIO_MODULE_DIR)
    set(PULSEAUDIO_MODULE_DIR "${CMAKE_INSTALL_PREFIX}/lib/pulse-${PULSEAUDIO_VERSION}/modules")
  else()
    set(PULSEAUDIO_MODULE_DIR "${SYSTEM_PULSEAUDIO_MODULE_DIR}")
  endif()
  message(STATUS "Using PULSEAUDIO_MODULE_DIR - ${PULSEAUDIO_MODULE_DIR}")
endif()

if(DOWNLOAD_PULSEAUDIO)
  include("cmake/download_pulseaudio.cmake")
else()
  if(PULSEAUDIO_DIR STREQUAL "")
    message(FATAL_ERROR "If DOWNLOAD_PULSEAUDIO is OFF, PULSEAUDIO_DIR should be defined")
  endif()

  get_filename_component(PULSEAUDIO_DIR "${PULSEAUDIO_DIR}" ABSOLUTE)

  message(STATUS "Using PULSEAUDIO_DIR - ${PULSEAUDIO_DIR}")

  include_directories(SYSTEM
    "${PULSEAUDIO_DIR}/src"
  )
endif()

if(DOWNLOAD_LIBTOOL)
  if(LIBTOOL_VERSION STREQUAL "")
    message(FATAL_ERROR "If DOWNLOAD_LIBTOOL is ON, LIBTOOL_VERSION should be defined")
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
  if(NOT ROC_REVISION)
    set(ROC_REVISION "v0.3.0")
  endif()
  message(STATUS "Using ROC_REVISION - ${ROC_REVISION}")

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

  link_libraries(
    "roc"
    "stdc++"
    "pthread"
  )
endif()

if((PULSEAUDIO_VERSION_MAJOR GREATER 14) OR
    ((PULSEAUDIO_VERSION_MAJOR EQUAL 14) AND
      NOT (PULSEAUDIO_VERSION_MINOR LESS 99)))
  set(SETUP_PULSEAUDIO 1)
  include("cmake/setup_pulseaudio.cmake")
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
    "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-variable")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

add_definitions(
  -DROC_PULSEAUDIO_VERSION=${PULSEAUDIO_VERSION}
)

include_directories("src")

add_library(rocpulse_helpers OBJECT
  "src/rocpulse_helpers.c"
)

if(SETUP_PULSEAUDIO)
  add_dependencies(rocpulse_helpers
    pulsebuild
  )
endif()

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

foreach(MODULE IN LISTS ALL_MODULES)
  install(
    FILES $<TARGET_FILE:${MODULE}>
    DESTINATION "${PULSEAUDIO_MODULE_DIR}"
  )
endforeach()

add_custom_target(compile_commands ALL
  DEPENDS ${ALL_MODULES}
  COMMENT "Copying compile_commands.json to project root"
  COMMAND "${CMAKE_COMMAND}" -E copy
    "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
    "${PROJECT_SOURCE_DIR}/compile_commands.json"
  )
