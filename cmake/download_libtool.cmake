include(ExternalProject)

ExternalProject_Add(libtool
  URL "ftp://ftp.gnu.org/gnu/libtool/libtool-${LIBTOOL_VERSION}.tar.gz"
  SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libtool-src"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  TEST_COMMAND ""
  LOG_DOWNLOAD ${USE_LOGFILES}
  LOG_PATCH ${USE_LOGFILES}
  LOG_CONFIGURE ${USE_LOGFILES}
  LOG_BUILD ${USE_LOGFILES}
  LOG_INSTALL ${USE_LOGFILES}
  LOG_TEST ${USE_LOGFILES}
)

include_directories(SYSTEM
  "${CMAKE_CURRENT_BINARY_DIR}/libtool-src/libltdl"
)
