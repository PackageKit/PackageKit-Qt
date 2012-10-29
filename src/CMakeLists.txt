# CMakeLists for PackageKit-Qt library

find_package (Qt4 REQUIRED)
include (${QT_USE_FILE})

include_directories(${CMAKE_BINARY_DIR}
		    ${CMAKE_CURRENT_BINARY_DIR}
		    ${CMAKE_CURRENT_SOURCE_DIR}
)

set (packagekitqt_HEADERS
		Daemon
		Transaction
		common.h
		daemon.h
                daemonprivate.h
		transaction.h
		transactionprivate.h
)

set (packagekitqt_SRC
		bitfield.cpp
		daemon.cpp
                daemonprivate.cpp
		transaction.cpp
		transactionprivate.cpp
)

qt4_automoc (${packagekitqt_SRC})
qt4_wrap_cpp (packagekitqt_MOC_SRC ${packagekitqt_HEADERS})

add_library(packagekit-qt2 SHARED ${packagekitqt_SRC} ${packagekitqt_HEADERS})

target_link_libraries(packagekit-qt2
		${QT_LIBRARIES}
)

add_subdirectory(modules)
