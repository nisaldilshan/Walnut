add_executable(IndexBuffers 
            main.cpp
            ../Common/Renderer2D.cpp
            ../Common/WebGPURenderer2D.cpp)

target_include_directories(IndexBuffers PRIVATE ../../Walnut/src)

target_link_libraries(IndexBuffers PRIVATE walnut)


