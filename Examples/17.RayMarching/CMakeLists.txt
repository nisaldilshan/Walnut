add_executable(RayMarching 
            main.cpp
            ../Common/Renderer2D.cpp
            ../Common/WebGPURenderer2D.cpp)

target_include_directories(RayMarching PRIVATE ../../Walnut/src)

target_link_libraries(RayMarching PRIVATE walnut)


