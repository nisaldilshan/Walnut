add_executable(TextureSampler 
            main.cpp
            ../Common/Renderer3D.cpp
            ../Common/WebGPURenderer3D.cpp)

target_include_directories(TextureSampler PRIVATE ../../Walnut/src)

target_link_libraries(TextureSampler PRIVATE walnut tinyobjloader)

target_compile_definitions(TextureSampler PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../Resources"
)


