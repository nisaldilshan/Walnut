vcpkg_from_github(
    OUT_SOURCE_VARIABLE SOURCE_PATH
    REPO nisaldilshan/Walnut
    REF "v${VERSION}"
    SHA512 0
    HEAD_REF master
)

# The application entry point relies on a global variable that is not exported from a DLL.
if(VCPKG_TARGET_IS_WINDOWS)
    vcpkg_check_linkage(ONLY_STATIC_LIBRARY)
endif()

# Walnut is built for exactly one renderer and one windowing system.
if("vulkan" IN_LIST FEATURES AND "opengl" IN_LIST FEATURES)
    message(FATAL_ERROR "walnut: features 'vulkan' and 'opengl' are mutually exclusive. Use \"default-features\": false to pick 'opengl'.")
elseif("opengl" IN_LIST FEATURES)
    set(WALNUT_RENDERER OpenGL)
elseif("vulkan" IN_LIST FEATURES)
    set(WALNUT_RENDERER Vulkan)
else()
    message(FATAL_ERROR "walnut: select a renderer feature, 'vulkan' or 'opengl'.")
endif()

if("glfw" IN_LIST FEATURES AND "sdl" IN_LIST FEATURES)
    message(FATAL_ERROR "walnut: features 'glfw' and 'sdl' are mutually exclusive. Use \"default-features\": false to pick 'sdl'.")
elseif("sdl" IN_LIST FEATURES)
    set(WALNUT_WINDOWING_SYSTEM SDL)
elseif("glfw" IN_LIST FEATURES)
    set(WALNUT_WINDOWING_SYSTEM GLFW)
else()
    message(FATAL_ERROR "walnut: select a windowing system feature, 'glfw' or 'sdl'.")
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DRENDERER=${WALNUT_RENDERER}
        -DWINDOWING_SYSTEM=${WALNUT_WINDOWING_SYSTEM}
        -DWALNUT_BUILD_EXAMPLES=OFF
        -DWALNUT_INSTALL=ON
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME walnut CONFIG_PATH lib/cmake/Walnut)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE.txt")
