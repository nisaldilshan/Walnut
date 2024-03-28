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
    FluidPlane(std::size_t size)
        : size(size)
    {
        s.resize(size * size);
        density.resize(size * size);
        Vx.resize(size * size);
        Vy.resize(size * size);
        Vx0.resize(size * size);
        Vy0.resize(size * size);
    }

    std::size_t size;
    float dt = 0.2f;
    float diff = 0.0f;
    float visc = 0.0000001f;
    
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
    FluidSolver2D() = default;
    ~FluidSolver2D() = default;
    void FluidSolveStep(FluidPlane& cube);
    void FluidPlaneAddDensity(FluidPlane& cube, int x, int y, float amount);
    void FluidPlaneAddVelocity(FluidPlane& cube, int x, int y, float amountX, float amountY);
private:
    
};


