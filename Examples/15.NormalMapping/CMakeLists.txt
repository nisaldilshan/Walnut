add_executable(NormalMapping
            main.cpp
)

target_link_libraries(NormalMapping PRIVATE RenderSys3D tinyobjloader)

target_compile_definitions(NormalMapping PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../Resources"
)


