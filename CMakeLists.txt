cmake_minimum_required(VERSION 3.6)
cmake_policy(SET CMP0048 NEW)

if (POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW)
endif()
if (POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW)
endif()

project(packagekit-qt VERSION 1.1.2)

# Used to set installation paths
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
include(GNUInstallDirs)

set(QPACKAGEKIT_API_LEVEL "1")

option(BUILD_WITH_QT6 "Use Qt6 instead of Qt5" OFF)

if(BUILD_WITH_QT6)
  set(QT_VERSION_MAJOR 6)
else()
  set(QT_VERSION_MAJOR 5)
endif()

find_package(Qt${QT_VERSION_MAJOR} 5.10 REQUIRED COMPONENTS Core DBus)

add_definitions(
    -DQT_NO_KEYWORDS
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_STRICT_ITERATORS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_USE_FAST_OPERATOR_PLUS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050900
)

# Forbid in-tree building
if(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
      message(STATUS "Please do an out-of-tree build:")
      message(STATUS "rm -f CMakeCache.txt && mkdir build && cd build; cmake .. && make")
      message(FATAL_ERROR "In-tree-build detected!")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
    "/usr" CACHE PATH "QPK default install prefix" FORCE)
endif()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

#
# Configure files
#
set (GETTEXT_PACKAGE "packagekit")
set (VERSION "${QPACKAGEKIT_VERSION}")
set (LOCALSTATEDIR "/var")
set (CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/packagekitqt${QT_VERSION_MAJOR}/")

add_definitions("-DLOCALSTATEDIR=\"${LOCALSTATEDIR}\"")
set (CMAKE_CXX_STANDARD 17)

configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)

#
# Custom C flags
#
add_definitions(
  "-Wformat"
  "-Wno-unused-parameter"
  "-Werror=empty-body"
  "-Werror=pointer-arith"
  "-Werror=init-self"
  "-Werror=missing-declarations"
  "-Werror=return-type"
  "-Werror=misleading-indentation"
  "-Werror=format-security"
)
option (MAINTAINER "Enable maintainer CFlags" OFF)
if (MAINTAINER)
    set(MAINTAINER_CFLAGS
      "-Werror"
      "-Wall"
      "-Wcast-align"
      "-Wno-uninitialized"
      "-Wempty-body"
      "-Winit-self"
      "-Wnull-dereference"
      "-Wfloat-equal"
      "-Winline"
    )
    add_definitions(${MAINTAINER_CFLAGS})
endif()

add_subdirectory(src)

install(EXPORT PackageKitQtTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE PackageKitQtTargets.cmake
        NAMESPACE PK::
        COMPONENT Devel)
