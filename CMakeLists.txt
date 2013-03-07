# CMakeLists for PackageKit-Qt
project(packagekit-qt)

cmake_minimum_required(VERSION 2.8.6 FATAL_ERROR)
find_package(PkgConfig REQUIRED)

set(CMAKE_BUILD_TYPE "Debug")

set(QPACKAGEKIT_VERSION_MAJOR  "0")
set(QPACKAGEKIT_VERSION_MINOR  "8")
set(QPACKAGEKIT_VERSION_PATCH  "8")
set(QPACKAGEKIT_VERSION_SUFFIX "${VERSION_SUFFIX}")
set(QPACKAGEKIT_VERSION "${QPACKAGEKIT_VERSION_MAJOR}.${QPACKAGEKIT_VERSION_MINOR}.${QPACKAGEKIT_VERSION_PATCH}")

set(QPACKAGEKIT_API_LEVEL "6")

# Forbid in-tree building
if(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
      message(STATUS "Please do an out-of-tree build:")
      message(STATUS "rm -f CMakeCache.txt && mkdir build && cd build; cmake .. && make")
      message(FATAL_ERROR "In-tree-build detected!")
endif(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})

#
# Options
#

# NONE

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
    "/usr" CACHE PATH "QPK default install prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#
# Configure files
#
set (PREFIXDIR "${CMAKE_INSTALL_PREFIX}")
set (DATADIR "${CMAKE_INSTALL_PREFIX}/share")
set (PKGDATADIR "${DATA_INSTALL_DIR}")
set (LIBDIR "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}")
set (PKGLIBDIR "${LIBDIR}/packagekit")
set (GETTEXT_PACKAGE "packagekit")
set (LOCALE_DIR "${DATADIR}/locale")
set (VERSION "${QPACKAGEKIT_VERSION}")
set (BUILDDIR "${CMAKE_BINARY_DIR}")
set (LOCALSTATEDIR "/var")

add_definitions("-DLOCALSTATEDIR=\"${LOCALSTATEDIR}\"")

set(CMAKE_INSTALL_LIBDIR "lib/${CMAKE_LIBRARY_ARCHITECTURE}" CACHE PATH "Output directory for libraries")

configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)

#
# Custom C flags
#
set (MAINTAINER_CFLAGS "-Werror -Wall -Wcast-align -Wno-uninitialized -Wempty-body -Wformat-security -Winit-self")
option (DISABLE_MAINTAINER_CFLAGS "Disable maintainer CFlags" ON)
if (DISABLE_MAINTAINER_CFLAGS)
	set (MAINTAINER_CFLAGS "")
endif (DISABLE_MAINTAINER_CFLAGS)
add_definitions(${MAINTAINER_CFLAGS})

add_subdirectory(src)
