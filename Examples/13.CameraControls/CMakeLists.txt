add_executable(CameraControls
            main.cpp
)

target_link_libraries(CameraControls PRIVATE RenderSys3D tinyobjloader)

target_compile_definitions(CameraControls PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../Resources"
)


