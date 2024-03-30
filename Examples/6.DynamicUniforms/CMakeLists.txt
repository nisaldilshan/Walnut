add_executable(DynamicUniforms 
            main.cpp
            ../Common/Renderer2D.cpp
            ../Common/WebGPURenderer2D.cpp)

target_include_directories(DynamicUniforms PRIVATE ../../Walnut/src)

target_link_libraries(DynamicUniforms PRIVATE walnut)

target_compile_definitions(DynamicUniforms PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)


