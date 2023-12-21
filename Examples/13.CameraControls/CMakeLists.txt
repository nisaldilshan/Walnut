add_executable(CameraControls
            main.cpp
            ../Common/Renderer3D.cpp
            ../Common/WebGPURenderer3D.cpp)

target_include_directories(CameraControls PRIVATE ../../Walnut/src)

target_link_libraries(CameraControls PRIVATE walnut tinyobjloader)

target_compile_definitions(CameraControls PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)


