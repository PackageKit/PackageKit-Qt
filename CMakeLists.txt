# CMakeLists for PackageKit-Qt
project(packagekit-qt)

cmake_minimum_required(VERSION 2.8.6 FATAL_ERROR)

set(CMAKE_BUILD_TYPE "Debug")

set(QPACKAGEKIT_VERSION_MAJOR  "0")
set(QPACKAGEKIT_VERSION_MINOR  "5")
set(QPACKAGEKIT_VERSION_PATCH  "7")
set(QPACKAGEKIT_VERSION_SUFFIX "${VERSION_SUFFIX}")
set(QPACKAGEKIT_VERSION "${QPACKAGEKIT_VERSION_MAJOR}.${QPACKAGEKIT_VERSION_MINOR}.${QPACKAGEKIT_VERSION_PATCH}")

set(QPACKAGEKIT_API_LEVEL "0")

# Forbid in-tree building
if(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
      message(STATUS "Please do an out-of-tree build:")
      message(STATUS "rm -f CMakeCache.txt && mkdir build && cd build; cmake .. && make")
      message(FATAL_ERROR "In-tree-build detected!")
endif(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})

#
# Options
#

#NONE

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
set (VERSION "${LISTALLER_VERSION}")
set (BUILDDIR "${CMAKE_BINARY_DIR}")

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
