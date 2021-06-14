include(ExternalProject)
include(ProcessorCount)

ProcessorCount(NUM_CPU)

if(DEFINED ENV{CI})
  set(USE_LOGFILES OFF)
else()
  set(USE_LOGFILES ON)
endif()

set(DEFAULT_PULSEAUDIO_VERSION "12.2")

if(NOT CMAKE_CROSSCOMPILING AND NOT SYSTEM_PULSEAUDIO_VERSION STREQUAL "")
  set(DEFAULT_PULSEAUDIO_VERSION "${SYSTEM_PULSEAUDIO_VERSION}")
endif()

set(LIBASOUND_VERSION "1.0.29" CACHE STRING "libasound version")
set(LIBSNDFILE_VERSION "1.0.28" CACHE STRING "libsndfile version")
set(LIBTOOL_VERSION "2.4.6" CACHE STRING "libtool version")
set(JSONC_VERSION "0.12-20140410" CACHE STRING "json-c version")

set(PULSEAUDIO_VERSION "${DEFAULT_PULSEAUDIO_VERSION}" CACHE STRING "pulseaudio version")

string(REPLACE "." ";" PULSEAUDIO_VERSION_COMPONENTS ${PULSEAUDIO_VERSION})

list(LENGTH PULSEAUDIO_VERSION_COMPONENTS PULSEAUDIO_VERSION_COMPONENTS_LENGTH)
while(PULSEAUDIO_VERSION_COMPONENTS_LENGTH LESS 3)
  list(APPEND PULSEAUDIO_VERSION_COMPONENTS 0)
  list(LENGTH PULSEAUDIO_VERSION_COMPONENTS PULSEAUDIO_VERSION_COMPONENTS_LENGTH)
endwhile()

list(GET PULSEAUDIO_VERSION_COMPONENTS 0 PULSEAUDIO_VERSION_MAJOR)
list(GET PULSEAUDIO_VERSION_COMPONENTS 1 PULSEAUDIO_VERSION_MINOR)
list(GET PULSEAUDIO_VERSION_COMPONENTS 2 PULSEAUDIO_VERSION_PATCH)

execute_process(
  COMMAND expr
   ${PULSEAUDIO_VERSION_MAJOR} * 1000000 +
   ${PULSEAUDIO_VERSION_MINOR} * 1000 +
   ${PULSEAUDIO_VERSION_PATCH}
  ERROR_QUIET
  OUTPUT_VARIABLE PULSEAUDIO_VERSION_CODE
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

foreach(VAR IN ITEMS
    LIBASOUND_VERSION LIBSNDFILE_VERSION LIBTOOL_VERSION JSONC_VERSION PULSEAUDIO_VERSION)
    message(STATUS "Using ${VAR} - ${${VAR}}")
endforeach()

ExternalProject_Add(libasound
  URL "ftp://ftp.alsa-project.org/pub/lib/alsa-lib-${LIBASOUND_VERSION}.tar.bz2"
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libasound-src"
  BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/libasound-build"
  CONFIGURE_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/libasound-src/configure"
    --host=${HOST}
    --enable-shared --disable-static --disable-python
    "CC=${CMAKE_C_COMPILER}"
    "CCLD=${CMAKE_C_COMPILER}"
  BUILD_COMMAND make -j${NUM_CPU}
  INSTALL_COMMAND ""
  TEST_COMMAND ""
  LOG_DOWNLOAD ${USE_LOGFILES}
  LOG_CONFIGURE ${USE_LOGFILES}
  LOG_BUILD ${USE_LOGFILES}
  )

ExternalProject_Add(libsndfile
  URL "http://www.mega-nerd.com/libsndfile/files/libsndfile-${LIBSNDFILE_VERSION}.tar.gz"
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libsndfile-src"
  BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/libsndfile-build"
  CONFIGURE_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/libsndfile-src/configure"
    --host=${HOST}
    --enable-static --disable-shared --disable-external-libs
    "CC=${CMAKE_C_COMPILER}"
    "CCLD=${CMAKE_C_COMPILER}"
    "CFLAGS=-w -fPIC -fvisibility=hidden"
  BUILD_COMMAND make -j${NUM_CPU}
  INSTALL_COMMAND ""
  TEST_COMMAND ""
  LOG_DOWNLOAD ${USE_LOGFILES}
  LOG_CONFIGURE ${USE_LOGFILES}
  LOG_BUILD ${USE_LOGFILES}
  )

ExternalProject_Add(libtool
  URL "ftp://ftp.gnu.org/gnu/libtool/libtool-${LIBTOOL_VERSION}.tar.gz"
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libtool-src"
  BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/libtool-build"
  CONFIGURE_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/libtool-src/configure"
    --host=${HOST}
    --enable-shared --disable-static
    "CC=${CMAKE_C_COMPILER}"
    "CCLD=${CMAKE_C_COMPILER}"
  BUILD_COMMAND make -j${NUM_CPU}
  INSTALL_COMMAND ""
  TEST_COMMAND ""
  LOG_DOWNLOAD ${USE_LOGFILES}
  LOG_CONFIGURE ${USE_LOGFILES}
  LOG_BUILD ${USE_LOGFILES}
  )

ExternalProject_Add(jsonc
  URL "https://github.com/json-c/json-c/archive/json-c-${JSONC_VERSION}.tar.gz"
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/jsonc-src"
  BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/jsonc-build"
  CONFIGURE_COMMAND env ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes
    "${CMAKE_CURRENT_BINARY_DIR}/jsonc-src/configure"
    --host=${HOST}
    --enable-static --disable-shared
    "CC=${CMAKE_C_COMPILER}"
    "CCLD=${CMAKE_C_COMPILER}"
    "CFLAGS=-w -fPIC -fvisibility=hidden"
  BUILD_COMMAND make # -j is buggy for json-c
  INSTALL_COMMAND ""
  TEST_COMMAND ""
  LOG_DOWNLOAD ${USE_LOGFILES}
  LOG_CONFIGURE ${USE_LOGFILES}
  LOG_BUILD ${USE_LOGFILES}
  )

set(PULSEAUDIO_INCLUDE_DIRS
  "-I${CMAKE_CURRENT_BINARY_DIR}/libasound-src/include"
  "-I${CMAKE_CURRENT_BINARY_DIR}/libsndfile-src/src"
  "-I${CMAKE_CURRENT_BINARY_DIR}/libsndfile-build/src"
  "-I${CMAKE_CURRENT_BINARY_DIR}/libtool-src/libltdl"
  "-I${CMAKE_CURRENT_BINARY_DIR}/jsonc-src"
  "-I${CMAKE_CURRENT_BINARY_DIR}/jsonc-build"
  )

string(REPLACE ";" " " PULSEAUDIO_INCLUDE_DIRS "${PULSEAUDIO_INCLUDE_DIRS}")

set(PULSEAUDIO_LINK_DIRS
  "-L${CMAKE_CURRENT_BINARY_DIR}/libasound-build/src/.libs"
  "-L${CMAKE_CURRENT_BINARY_DIR}/libsndfile-build/src/.libs"
  "-L${CMAKE_CURRENT_BINARY_DIR}/libtool-build/libltdl/.libs"
  "-L${CMAKE_CURRENT_BINARY_DIR}/jsonc-build/.libs"
  )

string(REPLACE ";" " " PULSEAUDIO_LINK_DIRS "${PULSEAUDIO_LINK_DIRS}")

set(PULSEAUDIO_PATCHES "")

if((PULSEAUDIO_VERSION_CODE GREATER_EQUAL 8099001) AND
    (PULSEAUDIO_VERSION_CODE LESS 11099001))
  list(APPEND PULSEAUDIO_PATCHES
    "patch/0001-memfd-wrappers-only-define-memfd_create-if-not-alrea.patch"
    )
endif()

set(PULSEAUDIO_PATCH_CMD "cd ${CMAKE_CURRENT_BINARY_DIR}/pulseaudio-src")

foreach(PATCH IN LISTS PULSEAUDIO_PATCHES)
  message(STATUS "Using ${PATCH}")
  set(PULSEAUDIO_PATCH_CMD "${PULSEAUDIO_PATCH_CMD} \
    && patch -p1 <${CMAKE_CURRENT_LIST_DIR}/../${PATCH}")
endforeach()

ExternalProject_Add(pulseaudio
  URL
   "https://freedesktop.org/software/pulseaudio/releases/pulseaudio-${PULSEAUDIO_VERSION}.tar.gz"
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/pulseaudio-src"
  BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/pulseaudio-build"
  PATCH_COMMAND sh -c "${PULSEAUDIO_PATCH_CMD}"
  CONFIGURE_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/pulseaudio-src/configure"
    --host=${HOST}
    --enable-shared
    --disable-static
    --disable-tests
    --disable-manpages
    --disable-orc
    --disable-webrtc-aec
    --disable-openssl
    --disable-neon-opt
    --without-caps
    "CC=${CMAKE_C_COMPILER}"
    "CCLD=${CMAKE_C_COMPILER}"
    "CFLAGS=-w -fomit-frame-pointer -O2 ${PULSEAUDIO_INCLUDE_DIRS}"
    "LDFLAGS=${PULSEAUDIO_LINK_DIRS}"
    "LIBJSON_CFLAGS= "
    "LIBJSON_LIBS=-ljson-c"
    "LIBSNDFILE_CFLAGS= "
    "LIBSNDFILE_LIBS=-lsndfile"
  BUILD_COMMAND make -j${NUM_CPU}
  INSTALL_COMMAND ""
  TEST_COMMAND ""
  LOG_DOWNLOAD ${USE_LOGFILES}
  LOG_PATCH ${USE_LOGFILES}
  LOG_CONFIGURE ${USE_LOGFILES}
  LOG_BUILD ${USE_LOGFILES}
  )

add_dependencies(jsonc libtool)
add_dependencies(libtool libsndfile)
add_dependencies(libsndfile libasound)

add_dependencies(pulseaudio
  libasound
  libsndfile
  libtool
  jsonc
  )

include_directories(SYSTEM
  "${CMAKE_CURRENT_BINARY_DIR}/libtool-src/libltdl"
  "${CMAKE_CURRENT_BINARY_DIR}/pulseaudio-src/src"
  "${CMAKE_CURRENT_BINARY_DIR}/pulseaudio-build"
  )
