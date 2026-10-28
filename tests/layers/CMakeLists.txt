# Pushes and pops layers while the application runs, see LayerTest.cpp.
# Opens a window like the render test, so it needs a display and a GPU driver too.

add_executable(walnut-layer-test LayerTest.cpp)
target_link_libraries(walnut-layer-test PRIVATE walnut)
# See tests/render/CMakeLists.txt
set_target_properties(walnut-layer-test PROPERTIES BUILD_WITH_INSTALL_RPATH OFF)

if(RENDERER STREQUAL "WebGPU")
    target_copy_webgpu_binaries(walnut-layer-test)
endif()
if(WIN32)
    add_custom_command(TARGET walnut-layer-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy -t "$<TARGET_FILE_DIR:walnut-layer-test>" "$<TARGET_RUNTIME_DLLS:walnut-layer-test>"
        COMMAND_EXPAND_LISTS
    )
endif()

add_test(NAME layers COMMAND walnut-layer-test)
set_tests_properties(layers PROPERTIES
    # The Vulkan renderer prints validation layer messages with this prefix
    FAIL_REGULAR_EXPRESSION "\\[vulkan validation\\]"
    TIMEOUT 120
)
