add_executable(FirstTriangle 
            main.cpp
            Renderer2D.cpp
            WebGPURenderer2D.cpp)

target_include_directories(FirstTriangle PRIVATE ../../Walnut/src)

target_link_libraries(FirstTriangle PRIVATE walnut)


