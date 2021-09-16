include(ExternalProject)

if(DEFINED ENV{CI})
  set(USE_LOGFILES OFF)
else()
  set(USE_LOGFILES ON)
endif()

set(SCONS_CMD
  scons
  --prefix=${CMAKE_CURRENT_BINARY_DIR}/roc-prefix
  --build-3rdparty=all
  --enable-static
  --disable-shared
  --disable-tools
  --disable-sox
  --disable-libunwind
  --disable-pulseaudio
  "CC=${CMAKE_C_COMPILER}"
  "CCLD=${CMAKE_C_COMPILER}"
  "CXX=${CMAKE_CXX_COMPILER}"
  "CXXLD=${CMAKE_CXX_COMPILER}"
  "AR=${CMAKE_AR}"
  "RANLIB=${CMAKE_RANLIB}"
  )

ExternalProject_Add(roc
  GIT_REPOSITORY "https://github.com/roc-streaming/roc-toolkit.git"
  GIT_TAG "develop"
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/roc-src"
  INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/roc-prefix"
  BUILD_IN_SOURCE ON
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ${SCONS_CMD}
  INSTALL_COMMAND ${SCONS_CMD} install
  TEST_COMMAND ""
  LOG_DOWNLOAD ${USE_LOGFILES}
  LOG_PATCH ${USE_LOGFILES}
  LOG_CONFIGURE ${USE_LOGFILES}
  LOG_BUILD ${USE_LOGFILES}
  LOG_INSTALL ${USE_LOGFILES}
  LOG_TEST ${USE_LOGFILES}
  )

include_directories(SYSTEM
  "${CMAKE_CURRENT_BINARY_DIR}/roc-prefix/include"
  )

set(LIBROC "${CMAKE_CURRENT_BINARY_DIR}/roc-prefix/lib/libroc.a")

set(CMAKE_SHARED_LINKER_FLAGS
  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--whole-archive ${LIBROC} -Wl,--no-whole-archive -lstdc++")
