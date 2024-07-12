find_package (OpenGL REQUIRED)
find_package(X11 REQUIRED)

# can not compile until we use SDL in android
# find_path(GLES2_INCLUDE_DIR GLES2/gl2.h HINTS ${ANDROID_NDK})
# find_library(GLES2_LIBRARY libGLESv2.so HINTS ${GLES2_INCLUDE_DIR}/../lib)

add_library (walnut-graphics-opengl STATIC
                    ImGuiBuildOpenGL.cpp
                    OpenGLGraphics.cpp
                    GlfwOpenGLRenderingBackend.cpp
                    OpenGLImage.cpp   
                )

target_include_directories(walnut-graphics-opengl PUBLIC ${CMAKE_SOURCE_DIR}/Walnut/src) 

#target_link_libraries(walnut-graphics-opengl PUBLIC glfw glad::glad imgui::imgui)
target_link_libraries(walnut-graphics-opengl PUBLIC SDL2::SDL2 glad::glad imgui::imgui)

message(STATUS "OPENGL_LIBRARIES= " ${OPENGL_LIBRARIES})
message(STATUS "X11_LIBRARIES= " ${X11_LIBRARIES})

# message(STATUS "GLES2_INCLUDE_DIR= " ${GLES2_INCLUDE_DIR})
# message(STATUS "GLES2_LIBRARY= " ${GLES2_LIBRARY})
target_link_libraries(walnut-graphics-opengl PUBLIC ${OPENGL_LIBRARIES} ${X11_LIBRARIES})

if (APPLE)
    target_link_libraries(walnut-graphics-opengl PUBLIC
        "-framework Metal"
        "-framework MetalKit"
        "-framework AppKit"
        "-framework Foundation"
        "-framework QuartzCore"
        "-framework IOSurface"
        "-framework Cocoa"
        "-framework CoreVideo"
        "-framework IOKit"
    )
endif ()

