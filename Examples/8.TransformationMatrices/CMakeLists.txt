add_executable(TransformationMatrices 
            main.cpp
)

target_link_libraries(TransformationMatrices PRIVATE RenderSys3D)

target_compile_definitions(TransformationMatrices PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)


