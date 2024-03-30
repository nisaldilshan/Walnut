add_executable(Lighting
            main.cpp
)

target_link_libraries(Lighting PRIVATE RenderSys3D tinyobjloader)

target_compile_definitions(Lighting PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../Resources"
)


