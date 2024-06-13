# CMakeLists for PackageKit-Qt library

set(packagekitqt_HEADERS
    PackageKitQt
    Daemon
    Transaction
    common.h
    daemon.h
    transaction.h
    Details
    details.h
    bitfield.h
    packagekitqt_global.h
    offline.h
    Offline
)

set(packagekitqt_SRC
    bitfield.cpp
    daemon.cpp
    daemonprivate.cpp
    transaction.cpp
    transactionprivate.cpp
    details.cpp
    offline.cpp
)

set(QPK_VERSION_HDR ${CMAKE_CURRENT_BINARY_DIR}/qpk-version.h)
configure_file(qpk-version.h.in ${QPK_VERSION_HDR})

find_path(PK_INTERFACES_DIR org.freedesktop.PackageKit.xml
	HINTS ${CMAKE_INSTALL_PREFIX} /usr
	PATH_SUFFIXES share/dbus-1/interfaces/
)
set(PK_INTERFACE_XML "${PK_INTERFACES_DIR}/org.freedesktop.PackageKit.xml")
set(PK_TRANSACTION_INTERFACE_XML "${PK_INTERFACES_DIR}/org.freedesktop.PackageKit.Transaction.xml")

if (NOT EXISTS "${PK_INTERFACE_XML}" OR NOT EXISTS "${PK_TRANSACTION_INTERFACE_XML}")
	message (FATAL_ERROR "Unable to find PackageKit DBus specifications! Please install PackageKit to continue!")
endif ()

set_source_files_properties(${PK_INTERFACE_XML} PROPERTIES NO_NAMESPACE true)
set_source_files_properties(${PK_TRANSACTION_INTERFACE_XML} PROPERTIES NO_NAMESPACE true)

if(${QT_VERSION_MAJOR} EQUAL 6)
qt6_add_dbus_interface(packagekitqt_SRC ${PK_INTERFACE_XML} daemonproxy)
qt6_add_dbus_interface(packagekitqt_SRC ${PK_TRANSACTION_INTERFACE_XML} transactionproxy)
else()
qt5_add_dbus_interface(packagekitqt_SRC ${PK_INTERFACE_XML} daemonproxy)
qt5_add_dbus_interface(packagekitqt_SRC ${PK_TRANSACTION_INTERFACE_XML} transactionproxy)
endif()

add_library(packagekitqt${QT_VERSION_MAJOR} ${packagekitqt_SRC} ${packagekitqt_HEADERS} ${packagekitqt_HEADERS_PRIVATE})
set_target_properties(packagekitqt${QT_VERSION_MAJOR} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${QPACKAGEKIT_API_LEVEL})

target_link_libraries(packagekitqt${QT_VERSION_MAJOR} PUBLIC Qt${QT_VERSION_MAJOR}::DBus)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packagekitqt${QT_VERSION_MAJOR}.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/packagekitqt${QT_VERSION_MAJOR}.pc
  @ONLY
)
target_include_directories(packagekitqt${QT_VERSION_MAJOR} INTERFACE "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/packagekitqt${QT_VERSION_MAJOR}/PackageKit/;${CMAKE_INSTALL_INCLUDEDIR}/packagekitqt${QT_VERSION_MAJOR}>")
install(TARGETS packagekitqt${QT_VERSION_MAJOR} EXPORT PackageKitQtTargets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/packagekitqt${QT_VERSION_MAJOR}.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
install(FILES ${packagekitqt_HEADERS} ${QPK_VERSION_HDR}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/packagekitqt${QT_VERSION_MAJOR}/PackageKit/
)

add_subdirectory(modules)
