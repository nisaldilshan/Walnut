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
        , dt(0.1f)
        , diffusion(0.0000001f)
        , viscosity(0.0000001f)
    {
        density0.resize(size * size);
        density.resize(size * size);
        Vx.resize(size * size);
        Vy.resize(size * size);
        Vx0.resize(size * size);
        Vy0.resize(size * size);
    }

    const std::size_t size;
    const float dt;
    const float diffusion;
    const float viscosity;
    
    std::vector<float> density0;
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


