add_executable(BasicCompute
            main.cpp
            ../Common/Compute.cpp
            ../Common/WebGPUCompute.cpp
)

target_include_directories(BasicCompute PRIVATE ../../Walnut/src)

target_link_libraries(BasicCompute PRIVATE walnut tinyobjloader)

target_compile_definitions(BasicCompute PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../Resources"
)


