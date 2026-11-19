# What provides the OpenGL functions: glad on the desktop, OpenGL ES 3 from the platform in the browser, on Android
# and on iOS.
if(EMSCRIPTEN)
    set(WALNUT_GL_LIBRARY) # WebGL comes with the link options below
elseif(ANDROID)
    set(WALNUT_GL_LIBRARY GLESv3) # system library, the NDK has its headers and the library to link to
elseif(IOS)
    set(WALNUT_GL_LIBRARY "-framework OpenGLES")
else()
    set(WALNUT_GL_LIBRARY glad::glad)
endif()

walnut_add_imgui_backend(walnut-imgui-opengl opengl3 ${WALNUT_GL_LIBRARY})
if(ANDROID OR IOS)
    target_compile_definitions(walnut-imgui-opengl PUBLIC IMGUI_IMPL_OPENGL_ES3)
endif()
if(IOS)
    # Apple deprecated OpenGL ES in iOS 12, but it still works
    target_compile_definitions(walnut-imgui-opengl PUBLIC GLES_SILENCE_DEPRECATION)
endif()

###################################################################################################################################

if(NOT EMSCRIPTEN AND NOT ANDROID AND NOT IOS)
    find_package (OpenGL REQUIRED)
    message(STATUS "OPENGL_LIBRARIES= " ${OPENGL_LIBRARIES})
    if(UNIX AND NOT APPLE)
        find_package(X11 REQUIRED)
        message(STATUS "X11_LIBRARIES= " ${X11_LIBRARIES})
    endif()
endif()


add_library (walnut-graphics-opengl STATIC
                    OpenGLGraphics.cpp
                    OpenGLImage.cpp
                    OpenGLRenderingBackend.cpp
                )

target_include_directories(walnut-graphics-opengl PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/Walnut/src>)
target_link_libraries(walnut-graphics-opengl PUBLIC ${WALNUT_GL_LIBRARY} imgui::imgui walnut-imgui-opengl)

if (WINDOWING_SYSTEM STREQUAL "SDL")
    target_sources(walnut-graphics-opengl PRIVATE SdlOpenGLRenderingBackend.cpp)
    target_link_libraries(walnut-graphics-opengl PRIVATE SDL3::SDL3)
else()
target_sources(walnut-graphics-opengl PRIVATE GlfwOpenGLRenderingBackend.cpp)
    target_link_libraries(walnut-graphics-opengl PUBLIC glfw)
endif()


if(EMSCRIPTEN)
    target_link_options(walnut-graphics-opengl PUBLIC -sUSE_WEBGL2=1 -sFULL_ES3=1)
elseif(NOT ANDROID AND NOT IOS)
    # Imported targets rather than library paths, so the installed package does not hard-code them.
    target_link_libraries(walnut-graphics-opengl PUBLIC OpenGL::GL)
    if(UNIX AND NOT APPLE)
        target_link_libraries(walnut-graphics-opengl PUBLIC X11::X11)
    endif()
endif()


if (APPLE AND NOT IOS) # AppKit and Cocoa are macOS only
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
