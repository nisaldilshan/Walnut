add_executable(BasicShading 
            main.cpp
            ../Common/Renderer3D.cpp
            ../Common/WebGPURenderer3D.cpp)

target_include_directories(BasicShading PRIVATE ../../Walnut/src)

target_link_libraries(BasicShading PRIVATE walnut tinyobjloader)

target_compile_definitions(BasicShading PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)


