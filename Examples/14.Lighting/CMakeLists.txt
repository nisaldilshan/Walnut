add_executable(Lighting
            main.cpp
            ../Common/Renderer3D.cpp
            ../Common/WebGPURenderer3D.cpp
            ../Common/Camera.cpp)

target_include_directories(Lighting PRIVATE ../../Walnut/src)

target_link_libraries(Lighting PRIVATE walnut tinyobjloader)

target_compile_definitions(Lighting PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../Resources"
)


