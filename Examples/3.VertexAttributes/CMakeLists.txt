add_executable(VertexAttributes 
            main.cpp
            ../Common/Renderer2D.cpp
            ../Common/WebGPURenderer2D.cpp)

target_include_directories(VertexAttributes PRIVATE ../../Walnut/src)

target_link_libraries(VertexAttributes PRIVATE walnut)


