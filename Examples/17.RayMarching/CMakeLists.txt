add_executable(RayMarching 
            main.cpp
            ../Common/Renderer3D.cpp
            ../Common/WebGPURenderer3D.cpp
            ../Common/Camera.cpp)

target_include_directories(RayMarching PRIVATE ../../Walnut/src)

target_link_libraries(RayMarching PRIVATE walnut)


