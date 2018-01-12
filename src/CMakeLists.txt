# CMakeLists for PackageKit-Qt library

find_package(Qt5 5.6 REQUIRED COMPONENTS Core DBus)

set(packagekitqt_HEADERS
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

find_path(PK_INTERFACES_DIR org.freedesktop.PackageKit.xml
	HINTS ${CMAKE_INSTALL_PREFIX} /usr
	PATH_SUFFIXES share/dbus-1/interfaces/
)
set(PK_INTERFACE_XML "${PK_INTERFACES_DIR}/org.freedesktop.PackageKit.xml")
set(PK_TRANSACTION_INTERFACE_XML "${PK_INTERFACES_DIR}/org.freedesktop.PackageKit.Transaction.xml")

if (NOT PK_INTERFACE_XML OR NOT PK_TRANSACTION_INTERFACE_XML)
	message (FATAL_ERROR "Unable to find PackageKit DBus specifications! Please install PackageKit to continue!")
endif ()

set_source_files_properties(${PK_INTERFACE_XML} PROPERTIES NO_NAMESPACE true)
set_source_files_properties(${PK_TRANSACTION_INTERFACE_XML} PROPERTIES NO_NAMESPACE true)

qt5_add_dbus_interface(packagekitqt_SRC ${PK_INTERFACE_XML} daemonproxy)
qt5_add_dbus_interface(packagekitqt_SRC ${PK_TRANSACTION_INTERFACE_XML} transactionproxy)

add_library(packagekitqt5 ${packagekitqt_SRC} ${packagekitqt_HEADERS} ${packagekitqt_HEADERS_PRIVATE})
set_target_properties(packagekitqt5 PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${QPACKAGEKIT_API_LEVEL})

target_link_libraries(packagekitqt5 PUBLIC Qt5::DBus)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packagekitqt5.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/packagekitqt5.pc
  @ONLY
)
target_include_directories(packagekitqt5 INTERFACE "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/packagekitqt5/PackageKit/;${CMAKE_INSTALL_INCLUDEDIR}/packagekitqt5>")
install(TARGETS packagekitqt5 EXPORT PackageKitQtTargets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/packagekitqt5.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
install(FILES ${packagekitqt_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/packagekitqt5/PackageKit/
)

add_subdirectory(modules)
