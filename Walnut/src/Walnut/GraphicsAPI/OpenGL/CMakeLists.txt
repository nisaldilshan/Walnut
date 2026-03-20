add_library(walnut-imgui-opengl STATIC
    ${imgui_INCLUDE_DIRS}/imgui.cpp
    ${imgui_INCLUDE_DIRS}/imgui_demo.cpp
    ${imgui_INCLUDE_DIRS}/imgui_draw.cpp
    ${imgui_INCLUDE_DIRS}/imgui_tables.cpp
    ${imgui_INCLUDE_DIRS}/imgui_widgets.cpp
    ${imgui_INCLUDE_DIRS}/imgui_impl_opengl3.cpp
)

if(ANDROID)
    target_compile_definitions(walnut-imgui-opengl PUBLIC IMGUI_IMPL_OPENGL_ES3)
else()
endif()

target_link_libraries(walnut-imgui-opengl PUBLIC imgui::imgui glad::glad)
if (WINDOWING_SYSTEM STREQUAL "SDL")
    target_sources(walnut-imgui-opengl PRIVATE ${imgui_INCLUDE_DIRS}/imgui_impl_sdl3.cpp)
    target_link_libraries(walnut-imgui-opengl PUBLIC SDL3::SDL3)
else()
    target_sources(walnut-imgui-opengl PRIVATE ${imgui_INCLUDE_DIRS}/imgui_impl_glfw.cpp)
    target_link_libraries(walnut-imgui-opengl PUBLIC glfw)
endif()

###################################################################################################################################

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
                    OpenGLGraphics.cpp
                    OpenGLImage.cpp  
                    OpenGLRenderingBackend.cpp 
                )

target_include_directories(walnut-graphics-opengl PUBLIC ${CMAKE_SOURCE_DIR}/Walnut/src) 
target_link_libraries(walnut-graphics-opengl PUBLIC glad::glad imgui::imgui walnut-imgui-opengl)

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

