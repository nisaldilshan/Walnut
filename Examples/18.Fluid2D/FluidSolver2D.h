#pragma once
#include <stdint.h>
#include <vector>

// Implementation of "Real-Time Fluid Dynamics for Games" paper by Jos Stam (GDC-2003)
// References :
    // https://www.dgp.toronto.edu/public_user/stam/reality/Research/pdf/GDC03.pdf
    // https://developer.nvidia.com/gpugems/gpugems/part-vi-beyond-triangles/chapter-38-fast-fluid-dynamics-simulation-gpu
    // https://mikeash.com/pyblog/fluid-simulation-for-dummies.html
    // https://www.youtube.com/watch?v=alhpH6ECFvQ

struct FluidPlane {
    int size;
    float dt;
    float diff;
    float visc;
    
    std::vector<float> s;
    std::vector<float> density;
    
    std::vector<float> Vx;
    std::vector<float> Vy;

    std::vector<float> Vx0;
    std::vector<float> Vy0;
};

class FluidSolver2D
{
public:
    FluidSolver2D(uint32_t size);
    ~FluidSolver2D();
private:
    void FluidSolveStep(FluidPlane& cube);
    void FluidPlaneAddDensity(int x, int y, float amount);
    void FluidPlaneAddVelocity(int x, int y, float amountX, float amountY);
};


