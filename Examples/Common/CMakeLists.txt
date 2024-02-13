cmake_minimum_required (VERSION 3.22.0)


project (RenderSys)


add_library (RenderSys2D STATIC
                Renderer2D.cpp
                WebGPURenderer2D.cpp
            )

target_include_directories(RenderSys2D PUBLIC ../../Walnut/src)
target_link_libraries(RenderSys2D PRIVATE walnut)


add_library (RenderSys3D STATIC
                Renderer3D.cpp
                WebGPURenderer3D.cpp
            )

target_include_directories(RenderSys3D PUBLIC ../../Walnut/src)
target_link_libraries(RenderSys3D PRIVATE walnut)