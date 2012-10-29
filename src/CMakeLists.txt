# CMakeLists for PackageKit-Qt library

find_package(Qt4 REQUIRED)

# Set up Qt4
set(QT_USE_QTDBUS TRUE)
set(QT_USE_QTSQL TRUE)
set(QT_DONT_USE_QTGUI TRUE)
include(${QT_USE_FILE})

include_directories(${CMAKE_BINARY_DIR}
		    ${CMAKE_CURRENT_BINARY_DIR}
		    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(packagekitqt_HEADERS
		Daemon
		Transaction
		common.h
		daemon.h
		transaction.h
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

qt4_automoc(${packagekitqt_SRC})
qt4_wrap_cpp(packagekitqt_MOC_SRC ${packagekitqt_HEADERS} ${packagekitqt_HEADERS_PRIVATE})

add_library(packagekit-qt2 SHARED ${packagekitqt_SRC} ${packagekitqt_HEADERS} ${packagekitqt_HEADERS_PRIVATE})
set_target_properties(packagekit-qt2 PROPERTIES VERSION ${QPACKAGEKIT_VERSION} SOVERSION ${QPACKAGEKIT_API_LEVEL})

target_link_libraries(packagekit-qt2
		${QT_LIBRARIES}
)

install(TARGETS packagekit-qt2 DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/packagekit-qt2.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
install(FILES ${packagekitqt_HEADERS}
	DESTINATION include/PackageKit/packagekit-qt2
)

add_subdirectory(modules)
