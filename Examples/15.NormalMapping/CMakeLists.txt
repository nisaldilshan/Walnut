add_executable(NormalMapping
            main.cpp
            ../Common/Renderer3D.cpp
            ../Common/WebGPURenderer3D.cpp
            ../Common/Camera.cpp)

target_include_directories(NormalMapping PRIVATE ../../Walnut/src)

target_link_libraries(NormalMapping PRIVATE walnut tinyobjloader)

target_compile_definitions(NormalMapping PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../Resources"
)


