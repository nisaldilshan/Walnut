add_executable(UniformBuffers 
            main.cpp
            ../Common/Renderer2D.cpp
            ../Common/WebGPURenderer2D.cpp)

target_include_directories(UniformBuffers PRIVATE ../../Walnut/src)

target_link_libraries(UniformBuffers PRIVATE walnut)

target_compile_definitions(UniformBuffers PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)


