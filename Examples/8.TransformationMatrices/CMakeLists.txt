add_executable(TransformationMatrices 
            main.cpp
            ../Common/Renderer3D.cpp
            ../Common/WebGPURenderer3D.cpp)

target_include_directories(TransformationMatrices PRIVATE ../../Walnut/src)

target_link_libraries(TransformationMatrices PRIVATE walnut)

target_compile_definitions(TransformationMatrices PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)


