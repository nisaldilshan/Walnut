find_package (OpenGL REQUIRED)
find_package(X11 REQUIRED)
find_package(Threads REQUIRED)

add_library (walnut-graphics-opengl STATIC
                    ImGuiBuildOpenGL.cpp
                    OpenGLGraphics.cpp
                    GlfwOpenGLRenderingBackend.cpp
                    OpenGLImage.cpp   
                )

target_include_directories(walnut-graphics-opengl PUBLIC ${CMAKE_SOURCE_DIR}/Walnut/src) 

target_link_libraries(walnut-graphics-opengl PUBLIC glfw3 glad)

message(STATUS "OPENGL_LIBRARIES= " ${OPENGL_LIBRARIES})
message(STATUS "X11_LIBRARIES= " ${X11_LIBRARIES})
target_link_libraries(walnut-graphics-opengl PUBLIC ${OPENGL_LIBRARIES} ${X11_LIBRARIES})
target_link_libraries(walnut-graphics-opengl PRIVATE Threads::Threads)

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

