add_executable(App src/WalnutApp.cpp)

target_include_directories(App PRIVATE ../Walnut/src)

target_link_libraries(App PRIVATE walnut imgui)

#set_target_properties(App PROPERTIES LINK_FLAGS " -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s USE_GLFW=3 -s USE_WEBGPU=1")