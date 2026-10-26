# Opens a window, renders a known scene and checks the pixels read back from it, see RenderTest.cpp.
# Needs a display and a GPU driver; CI runs it on Linux under Xvfb with Mesa's software renderers.

add_executable(walnut-render-test RenderTest.cpp)
target_link_libraries(walnut-render-test PRIVATE walnut stb::stb)
# The root CMakeLists.txt makes the build tree use the install RPATH, which only works on macOS.
# The build tree RPATH points at walnut and the package manager's shared libraries (e.g. the Vulkan loader).
set_target_properties(walnut-render-test PROPERTIES BUILD_WITH_INSTALL_RPATH OFF)

if(RENDERER STREQUAL "WebGPU")
    target_copy_webgpu_binaries(walnut-render-test)
endif()
if(WIN32)
    add_custom_command(TARGET walnut-render-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t "$<TARGET_FILE_DIR:walnut-render-test>" "$<TARGET_RUNTIME_DLLS:walnut-render-test>"
        COMMAND_EXPAND_LISTS
    )
endif()

add_test(NAME render COMMAND walnut-render-test)
set_tests_properties(render PROPERTIES
    SKIP_RETURN_CODE 77
    # The Vulkan renderer prints validation layer messages with this prefix
    FAIL_REGULAR_EXPRESSION "\\[vulkan validation\\]"
    TIMEOUT 120
)
