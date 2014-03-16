# CMakeLists for PackageKit-Qt library

find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Sql REQUIRED)

# Set up Qt5
set(QT_USE_QTDBUS TRUE)
set(QT_USE_QTSQL TRUE)
set(QT_DONT_USE_QTGUI TRUE)

set(CMAKE_AUTOMOC TRUE)

include_directories(${CMAKE_BINARY_DIR}
		    ${CMAKE_CURRENT_BINARY_DIR}
		    ${CMAKE_CURRENT_SOURCE_DIR}
                    ${Qt5Core_INCLUDE_DIRS}
                    ${Qt5DBus_INCLUDE_DIRS}
                    ${Qt5Sql_INCLUDE_DIRS}
)

set(packagekitqt_HEADERS
    Daemon
    Transaction
    common.h
    daemon.h
    transaction.h
    bitfield.h
)

set(packagekitqt_HEADERS_PRIVATE
                daemonprivate.h
		transactionprivate.h
)

set(packagekitqt_SRC
		bitfield.cpp
		daemon.cpp
                daemonprivate.cpp
		transaction.cpp
		transactionprivate.cpp
)

find_file(PK_INTERFACE_XML org.freedesktop.PackageKit.xml
	PATHS ${CMAKE_INSTALL_PREFIX}/share/dbus-1/interfaces/
)
find_file(PK_TRANSACTION_INTERFACE_XML org.freedesktop.PackageKit.Transaction.xml
	PATHS ${CMAKE_INSTALL_PREFIX}/share/dbus-1/interfaces/
)
if (PK_INTERFACE_XML STREQUAL "PK_INTERFACE_XML-NOTFOUND" OR PK_TRANSACTION_INTERFACE_XML STREQUAL "PK_TRANSACTION_INTERFACE_XML-NOTFOUND")
	message (FATAL_ERROR "Unable to find PackageKit DBus specifications! Please install PackageKit to continue!")
endif ()

set_source_files_properties(${PK_INTERFACE_XML} PROPERTIES CLASSNAME DaemonProxy)
qt5_add_dbus_interface(packagekitqt_SRC ${PK_INTERFACE_XML} daemonproxy)

set_source_files_properties(${PK_TRANSACTION_INTERFACE_XML} PROPERTIES CLASSNAME TransactionProxy)
qt5_add_dbus_interface(packagekitqt_SRC ${PK_TRANSACTION_INTERFACE_XML} transactionproxy)

qt5_wrap_cpp(packagekitqt_MOC_SRC ${packagekitqt_HEADERS} ${packagekitqt_HEADERS_PRIVATE})

add_library(packagekit-qt2 SHARED ${packagekitqt_SRC} ${packagekitqt_HEADERS} ${packagekitqt_HEADERS_PRIVATE})
set_target_properties(packagekit-qt2 PROPERTIES VERSION ${QPACKAGEKIT_VERSION} SOVERSION ${QPACKAGEKIT_API_LEVEL})

target_link_libraries(packagekit-qt2
		${QT_LIBRARIES}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packagekit-qt2.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/packagekit-qt2.pc
  @ONLY
)

install(TARGETS packagekit-qt2 DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/packagekit-qt2.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
install(FILES ${packagekitqt_HEADERS}
	DESTINATION include/PackageKit/packagekit-qt2
)

add_subdirectory(modules)
