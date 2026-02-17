if(ANDROID)
    find_path(GLES3_INCLUDE_DIR GLES3/gl3.h HINTS ${ANDROID_NDK})
    find_library(GLES3_LIBRARY libGLESv3.so HINTS ${GLES3_INCLUDE_DIR}/../lib)
    message(STATUS "GLES3_INCLUDE_DIR= " ${GLES3_INCLUDE_DIR})
    message(STATUS "GLES3_LIBRARY= " ${GLES3_LIBRARY})
else()
    find_package (OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    message(STATUS "OPENGL_LIBRARIES= " ${OPENGL_LIBRARIES})
    message(STATUS "X11_LIBRARIES= " ${X11_LIBRARIES})
endif()


add_library (walnut-graphics-opengl STATIC
                    ImGuiBuildOpenGL.cpp
                    OpenGLGraphics.cpp
                    OpenGLImage.cpp   
                )

target_include_directories(walnut-graphics-opengl PUBLIC ${CMAKE_SOURCE_DIR}/Walnut/src) 
target_link_libraries(walnut-graphics-opengl PUBLIC glad::glad imgui::imgui)

if (WINDOWING_SYSTEM STREQUAL "SDL")
    target_sources(walnut-graphics-opengl PRIVATE SdlOpenGLRenderingBackend.cpp)
    target_link_libraries(walnut-graphics-opengl PRIVATE SDL3::SDL3)
else()
target_sources(walnut-graphics-opengl PRIVATE GlfwOpenGLRenderingBackend.cpp)
    target_link_libraries(walnut-graphics-opengl PUBLIC glfw)
endif()


if(ANDROID)
    target_link_libraries(walnut-graphics-opengl PUBLIC ${GLES3_LIBRARY})
elseif(IOS)
else()
    target_link_libraries(walnut-graphics-opengl PUBLIC ${OPENGL_LIBRARIES} ${X11_LIBRARIES})
endif()


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

