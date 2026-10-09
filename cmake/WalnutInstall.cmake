# Install rules and the Walnut CMake package (find_package(Walnut CONFIG) -> Walnut::walnut),
# used by the vcpkg port.

if(RENDERER STREQUAL "WebGPU")
    message(STATUS "Walnut install rules are not generated for the WebGPU renderer")
    return()
endif()

include(CMakePackageConfigHelpers)

install(TARGETS walnut walnut-graphics-${WALNUT_RENDERER_NAME} walnut-imgui-${WALNUT_RENDERER_NAME}
    EXPORT WalnutTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/Walnut/src/Walnut
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
    PATTERN "ImGui" EXCLUDE # only holds the embedded font
)

set(WALNUT_CMAKE_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/Walnut)
install(EXPORT WalnutTargets
    NAMESPACE Walnut::
    DESTINATION ${WALNUT_CMAKE_INSTALL_DIR}
)
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/WalnutConfig.cmake.in
    ${PROJECT_BINARY_DIR}/WalnutConfig.cmake
    INSTALL_DESTINATION ${WALNUT_CMAKE_INSTALL_DIR}
)
write_basic_package_version_file(${PROJECT_BINARY_DIR}/WalnutConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${PROJECT_BINARY_DIR}/WalnutConfig.cmake
    ${PROJECT_BINARY_DIR}/WalnutConfigVersion.cmake
    DESTINATION ${WALNUT_CMAKE_INSTALL_DIR}
)
