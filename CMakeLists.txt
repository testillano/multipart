############
# Versions #
############
cmake_minimum_required (VERSION 3.5.1)
project(ert_multipart LANGUAGES CXX)

set(MAIN_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(MAIN_PROJECT ON)
endif()

#############
# Variables #
#############
option(ERT_MULTIPART_BuildExamples "Build the examples." ${MAIN_PROJECT})
set(ERT_MULTIPART_TARGET_NAME       ${PROJECT_NAME})
set(ERT_MULTIPART_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/include")

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

###########
# Modules #
###########
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")

# Operative system
include(os_type)
set_cmake_os_type()

# Build type
include(build_type)
set_cmake_build_type()

# Build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/lib)
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY is ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "CMAKE_LIBRARY_OUTPUT_DIRECTORY is ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "CMAKE_ARCHIVE_OUTPUT_DIRECTORY is ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

# Compilation flags
include(compiler_flags)
set_cmake_compiler_flags()

################
# Dependencies #
################
#include(FetchContent)
#
#FetchContent_Declare(ert_logger
#  GIT_REPOSITORY https://github.com/testillano/logger.git
#  GIT_TAG v1.0.4)
#message(STATUS  "Fetching ert_logger ... this may take a while")
#FetchContent_GetProperties(ert_logger)
#
#if(NOT ert_logger_POPULATED)
#  FetchContent_Populate(ert_logger)
#  add_subdirectory(${ert_logger_SOURCE_DIR} ${ert_logger_BINARY_DIR} EXCLUDE_FROM_ALL)
#endif()
#
#FetchContent_Declare(tatsuhiro_nghttp2
#  GIT_REPOSITORY https://github.com/nghttp2/nghttp2
#  GIT_TAG v1.43.0)
#message(STATUS  "Fetching tatsuhiro_nghttp2 ... this may take a while")
#FetchContent_GetProperties(tatsuhiro_nghttp2)
#
#if(NOT tatsuhiro_nghttp2)
#  FetchContent_Populate(tatsuhiro_nghttp2)
#  add_subdirectory(${tatsuhiro_nghttp2_SOURCE_DIR} ${tatsuhiro_nghttp2_BINARY_DIR} EXCLUDE_FROM_ALL)
#endif()

#include(ExternalProject)
#ExternalProject_Add(tatsuhiro_nghttp2
#  GIT_REPOSITORY    https://github.com/nghttp2/nghttp2
#  GIT_TAG           v1.43.0
#)

##################
# Subdirectories #
##################
add_subdirectory( src )
if (ERT_MULTIPART_BuildExamples)
  add_subdirectory( examples )
endif()

###########
# Install #
###########
#SET(MY_OWN_INSTALL_PREFIX "/opt/ert_multipart" CACHE PATH "Prefix prepended to install directories")
#SET(CMAKE_INSTALL_PREFIX "${MY_OWN_INSTALL_PREFIX}" CACHE INTERNAL "Prefix prepended to install directories" FORCE)

include(GNUInstallDirs)
message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
install(
          DIRECTORY ${ERT_MULTIPART_INCLUDE_BUILD_DIR}/ert/multipart
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ert
)

# Library install, at src project

###########
# DOXYGEN #
###########
find_package(Doxygen)
IF(Doxygen_FOUND)
  add_custom_target(doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen
    COMMENT "Generating API documentation with Doxygen" VERBATIM
  )
ENDIF(Doxygen_FOUND)

