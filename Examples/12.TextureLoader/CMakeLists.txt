add_executable(TextureLoader
            main.cpp
            ../Common/Renderer3D.cpp
            ../Common/WebGPURenderer3D.cpp)

target_include_directories(TextureLoader PRIVATE ../../Walnut/src)

target_link_libraries(TextureLoader PRIVATE walnut tinyobjloader)

target_compile_definitions(TextureLoader PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../Resources"
)


