# This is only meant to be included as a subdirectory in another project.
# It assumes that targets 'glfw' and 'webgpu' exist.
# Loot at examples/CMakeLists.txt to see how to use it in a project.

# The glfw3webgpu target
add_library(glfw3webgpu STATIC glfw3webgpu.c)
target_include_directories(glfw3webgpu PUBLIC .)

if (WINDOWING_SYSTEM STREQUAL "SDL")
    target_link_libraries(glfw3webgpu PUBLIC webgpu SDL3::SDL3)
else()
    target_link_libraries(glfw3webgpu PUBLIC webgpu glfw)
endif()

if (APPLE)
  target_compile_options(glfw3webgpu PRIVATE -x objective-c)
  target_link_libraries(glfw3webgpu PRIVATE "-framework Cocoa" "-framework CoreVideo" "-framework IOKit" "-framework QuartzCore")
endif ()