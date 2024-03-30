add_executable(Fluid2D 
            FluidSolver2D.cpp
            main.cpp
)

target_link_libraries(Fluid2D PRIVATE RenderSys2D)


