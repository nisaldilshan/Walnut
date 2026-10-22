if(CMAKE_SYSTEM_NAME MATCHES "Android")
add_library(App SHARED src/WalnutApp.cpp)
else()
add_executable(App src/WalnutApp.cpp )
endif()

if(CMAKE_SYSTEM_NAME MATCHES "iOS")
target_sources(App PRIVATE 
                src/SDL_uikit_main.c
)
set_target_properties(App PROPERTIES BUNDLE TRUE)
endif()

# walnut brings in its include paths and imgui.
target_link_libraries(App PRIVATE walnut)

if (WINDOWING_SYSTEM STREQUAL "SDL")
    target_link_libraries(App PRIVATE SDL3::SDL3) # SDL2::SDL2main
else()
    target_link_libraries(App PUBLIC glfw)
endif()

if (IS_EMSCRIPTEN)
    #set_target_properties(App PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s USE_WEBGPU=1")
    # Emit index.html (plus index.js/index.wasm) using shell.html as the page template,
    # so a static server pointed at the build folder serves the app at its root URL.
    set(APP_SHELL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/shell.html")
    set_target_properties(App PROPERTIES
        OUTPUT_NAME index
        SUFFIX ".html"
        LINK_DEPENDS "${APP_SHELL_FILE}"
        LINK_FLAGS
        " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s GL_DEBUG=1 -s GL_ASSERTIONS=1 -s SAFE_HEAP=1 --shell-file \"${APP_SHELL_FILE}\"")
endif()

function(copy_runtime_dlls_at_build targetName)
    if(WIN32)
        add_custom_command(
            TARGET ${targetName}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy -t "$<TARGET_FILE_DIR:${targetName}>" "$<TARGET_RUNTIME_DLLS:${targetName}>"
            COMMENT "Copying runtime DLLs for ${targetName}"
            USES_TERMINAL
            COMMAND_EXPAND_LISTS
        )
    endif()
endfunction()

copy_runtime_dlls_at_build(App)