add_executable(FirstTexture 
            main.cpp
            ../Common/Renderer3D.cpp
            ../Common/WebGPURenderer3D.cpp)

target_include_directories(FirstTexture PRIVATE ../../Walnut/src)

target_link_libraries(FirstTexture PRIVATE walnut tinyobjloader)

target_compile_definitions(FirstTexture PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)


