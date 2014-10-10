# CMakeLists for PackageKit-Qt
project(packagekit-qt)

cmake_minimum_required(VERSION 2.8.6 FATAL_ERROR)
find_package(PkgConfig REQUIRED)

set(CMAKE_BUILD_TYPE "Debug")

# Used to set installation paths
include(GNUInstallDirs)

set(QPACKAGEKIT_VERSION_MAJOR  "0")
set(QPACKAGEKIT_VERSION_MINOR  "9")
set(QPACKAGEKIT_VERSION_PATCH  "5")
set(QPACKAGEKIT_VERSION_SUFFIX "${VERSION_SUFFIX}")
set(QPACKAGEKIT_VERSION "${QPACKAGEKIT_VERSION_MAJOR}.${QPACKAGEKIT_VERSION_MINOR}.${QPACKAGEKIT_VERSION_PATCH}")

# Forbid in-tree building
if(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
      message(STATUS "Please do an out-of-tree build:")
      message(STATUS "rm -f CMakeCache.txt && mkdir build && cd build; cmake .. && make")
      message(FATAL_ERROR "In-tree-build detected!")
endif(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})

#
# Options
#
option(USE_QT5 "Build against Qt5 instead of Qt4." OFF)
if(USE_QT5)
  set(QPACKAGEKIT_API_LEVEL "0")
  set(LIBNAME "packagekitqt5")
  set(QT_VERSION "5")
else()
  set(QPACKAGEKIT_API_LEVEL "0")
  set(LIBNAME "packagekitqt4")
  set(QT_VERSION "4")
endif(USE_QT5)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
    "/usr" CACHE PATH "QPK default install prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

#
# Configure files
#
set (GETTEXT_PACKAGE "packagekit")
set (VERSION "${QPACKAGEKIT_VERSION}")
set (LOCALSTATEDIR "/var")
set (CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${LIBNAME}/")

add_definitions("-DLOCALSTATEDIR=\"${LOCALSTATEDIR}\"")
add_definitions("-std=gnu++11")

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

install(EXPORT PackageKitQtTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE PackageKitQtTargets.cmake
        NAMESPACE PK::
        COMPONENT Devel)
