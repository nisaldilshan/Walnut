add_executable(First3DShape 
            main.cpp
            ../Common/Renderer3D.cpp
            ../Common/WebGPURenderer3D.cpp)

target_include_directories(First3DShape PRIVATE ../../Walnut/src)

target_link_libraries(First3DShape PRIVATE walnut)

target_compile_definitions(First3DShape PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)


