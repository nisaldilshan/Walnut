if(ANDROID)
    find_path(GLES3_INCLUDE_DIR GLES3/gl3.h HINTS ${ANDROID_NDK})
    find_library(GLES3_LIBRARY libGLESv3.so HINTS ${GLES3_INCLUDE_DIR}/../lib)
    message(STATUS "GLES3_INCLUDE_DIR= " ${GLES3_INCLUDE_DIR})
    message(STATUS "GLES3_LIBRARY= " ${GLES3_LIBRARY})
else()

find_package (OpenGL REQUIRED)
message(STATUS "OPENGL_LIBRARIES= " ${OPENGL_LIBRARIES})
if (EMSCRIPTEN)
else()
    find_package(glad REQUIRED)
    find_package(X11 REQUIRED)
    message(STATUS "X11_LIBRARIES= " ${X11_LIBRARIES})
endif()
endif()


add_library (walnut-graphics-opengl STATIC
                    ImGuiBuildOpenGL.cpp
                    OpenGLGraphics.cpp
                    OpenGLImage.cpp   
                )

target_include_directories(walnut-graphics-opengl PUBLIC ${CMAKE_SOURCE_DIR}/Walnut/src) 
target_link_libraries(walnut-graphics-opengl PUBLIC imgui::imgui)

if (WINDOWING_SYSTEM STREQUAL "SDL")
    target_sources(walnut-graphics-opengl PRIVATE SdlOpenGLRenderingBackend.cpp)
    target_link_libraries(walnut-graphics-opengl PUBLIC SDL2::SDL2)
else()
target_sources(walnut-graphics-opengl PRIVATE GlfwOpenGLRenderingBackend.cpp)
    target_link_libraries(walnut-graphics-opengl PUBLIC glfw)
endif()


if(ANDROID)
    target_link_libraries(walnut-graphics-opengl PUBLIC ${GLES3_LIBRARY})
else()
    target_link_libraries(walnut-graphics-opengl PUBLIC ${OPENGL_LIBRARIES} ${X11_LIBRARIES})
    set_target_properties(walnut-graphics-opengl PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3")
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

