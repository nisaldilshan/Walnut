#include "FluidSolver.h"

FluidSolver::FluidSolver(uint32_t size)
{
}

FluidSolver::~FluidSolver()
{
}

void FluidSolver::FluidCubeAddDensity(int x, int y, int z, float amount)
{
    int N = cube->size;
    cube->density[IX(x, y, z)] += amount;
}

void FluidSolver::FluidCubeAddVelocity(int x, int y, int z, float amountX, float amountY, float amountZ)
{
    int N = cube->size;
    int index = IX(x, y, z);
    
    cube->Vx[index] += amountX;
    cube->Vy[index] += amountY;
    cube->Vz[index] += amountZ;
}