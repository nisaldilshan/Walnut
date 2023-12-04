find_package (OpenGL REQUIRED)

add_library (walnut-graphics-opengl STATIC
                    ImGuiBuildOpenGL.cpp
                    OpenGLGraphics.cpp
                    GlfwOpenGLRenderingBackend.cpp
                    OpenGLImage.cpp   
                )

target_link_libraries(walnut-graphics-opengl PUBLIC glfw3 glad)

message(STATUS "OPENGL_LIBRARIES= " ${OPENGL_LIBRARIES})
target_link_libraries(walnut-graphics-opengl PUBLIC ${OPENGL_LIBRARIES})

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

