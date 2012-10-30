# CMakeLists for PackageKit-Qt lib extra modules

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packagekit-qt2-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/packagekit-qt2-config.cmake
  @ONLY
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packagekit-qt2-config-version.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/packagekit-qt2-config-version.cmake
  @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/packagekit-qt2-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/packagekit-qt2-config-version.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/packagekit-qt2/
)
