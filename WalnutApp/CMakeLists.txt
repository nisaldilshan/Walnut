add_executable(App src/WalnutApp.cpp)

target_include_directories(App PRIVATE ../Walnut/src)
target_link_libraries(App PRIVATE glm::glm imgui::imgui)
if (WIN32)
target_link_libraries(App PRIVATE walnut)

else()
target_link_libraries(App PRIVATE walnut) #  "--shell-file /Users/nisal/dev/other/cpp/Walnut/WalnutApp/src/shell_minimal.html"
endif()

if (WINDOWING_SYSTEM STREQUAL "SDL")
    target_link_libraries(App PRIVATE SDL2::SDL2)
else()
    target_link_libraries(App PUBLIC glfw)
endif()

if (IS_EMSCRIPTEN)
    #set_target_properties(App PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s USE_WEBGPU=1")
    set_target_properties(App PROPERTIES LINK_FLAGS 
        " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s GL_DEBUG=1 -s DEMANGLE_SUPPORT=1 -s GL_ASSERTIONS=1 -s SAFE_HEAP=1")
endif()

