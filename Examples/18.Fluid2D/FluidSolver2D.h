#pragma once
#include <stdint.h>

// Implementation of "Real-Time Fluid Dynamics for Games" paper by Jos Stam (GDC-2003)
// References :
    // https://www.dgp.toronto.edu/public_user/stam/reality/Research/pdf/GDC03.pdf
    // https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-38-fast-fluid-dynamics-simulation-gpu
    // https://mikeash.com/pyblog/fluid-simulation-for-dummies.html
    // https://www.youtube.com/watch?v=alhpH6ECFvQ

class FluidSolver2D
{
public:
    FluidSolver2D(uint32_t size);
    ~FluidSolver2D();
private:
    void FluidCubeAddDensity(int x, int y, int z, float amount);
    void FluidCubeAddVelocity(int x, int y, int z, float amountX, float amountY, float amountZ);
};


