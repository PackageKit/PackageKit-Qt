include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/packagekit-qt-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/packagekitqt5-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/packagekitqt5/
)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/packagekitqt5-config-version.cmake VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/packagekitqt5-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/packagekitqt5-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/packagekitqt5/
)
