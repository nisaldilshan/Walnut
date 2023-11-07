add_executable(VertexBuffers 
            main.cpp
            ../Common/Renderer2D.cpp
            ../Common/WebGPURenderer2D.cpp)

target_include_directories(VertexBuffers PRIVATE ../../Walnut/src)

target_link_libraries(VertexBuffers PRIVATE walnut)


