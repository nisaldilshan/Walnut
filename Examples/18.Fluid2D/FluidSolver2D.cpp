#include "FluidSolver2D.h"

#define IX(x, y) ((x) + (y) * N)

FluidSolver2D::FluidSolver2D(uint32_t size)
{
}

FluidSolver2D::~FluidSolver2D()
{
}

static void set_bnd(int b, float *x, int N)
{
    //set edges
    for(int i = 1; i < N - 1; i++) {
        x[IX(i, 0  )] = b == 2 ? -x[IX(i, 1  )] : x[IX(i, 1  )];
        x[IX(i, N-1)] = b == 2 ? -x[IX(i, N-2)] : x[IX(i, N-2)];
    }
    for(int j = 1; j < N - 1; j++) {
        x[IX(0  , j)] = b == 1 ? -x[IX(1  , j)] : x[IX(1  , j)];
        x[IX(N-1, j)] = b == 1 ? -x[IX(N-2, j)] : x[IX(N-2, j)];
    }
    
    // set corners
    x[IX(0, 0)]         = 0.5f * (x[IX(1, 0)]
                                + x[IX(0, 1)]);
    x[IX(0, N-1)]       = 0.5f * (x[IX(1, N-1)]
                                + x[IX(0, N-2)]);
    x[IX(N-1, 0)]       = 0.5f * (x[IX(N-2, 0)]
                                + x[IX(N-1, 1)]);
    x[IX(N-1, N-1)]     = 0.5f * (x[IX(N-2, N-1)]
                                + x[IX(N-1, N-2)]);
}

static void lin_solve(int b, float *x, float *x0, float a, float c, int iter, int N)
{
    float cRecip = 1.0 / c;
    for (int k = 0; k < iter; k++) {
        for (int j = 1; j < N - 1; j++) {
            for (int i = 1; i < N - 1; i++) {
                x[IX(i, j)] =
                    (x0[IX(i, j)]
                        + a*(    x[IX(i+1, j  )]
                                +x[IX(i-1, j  )]
                                +x[IX(i  , j+1)]
                                +x[IX(i  , j-1)]
                        )) * cRecip;
            }
        }
        set_bnd(b, x, N);
    }
}

static void diffuse (int b, float *x, float *x0, float diff, float dt, int iter, int N)
{
    float a = dt * diff * (N - 2) * (N - 2);
    lin_solve(b, x, x0, a, 1 + 6 * a, iter, N);
}

void FluidSolver2D::FluidSolveStep(FluidPlane& cube)
{
    int N          = cube.size;
    float visc     = cube.visc;
    float diff     = cube.diff;
    float dt       = cube.dt;
    float *Vx      = cube.Vx;
    float *Vy      = cube.Vy;
    float *Vx0     = cube.Vx0;
    float *Vy0     = cube.Vy0;
    float *s       = cube.s;
    float *density = cube.density;
    
    diffuse(1, Vx0, Vx, visc, dt, 4, N);
    diffuse(2, Vy0, Vy, visc, dt, 4, N);
    
    // project(Vx0, Vy0, Vz0, Vx, Vy, 4, N);
    
    // advect(1, Vx, Vx0, Vx0, Vy0, Vz0, dt, N);
    // advect(2, Vy, Vy0, Vx0, Vy0, Vz0, dt, N);
    // advect(3, Vz, Vz0, Vx0, Vy0, Vz0, dt, N);
    
    // project(Vx, Vy, Vz, Vx0, Vy0, 4, N);
    
    diffuse(0, s, density, diff, dt, 4, N);
    // advect(0, density, s, Vx, Vy, Vz, dt, N);
}

void FluidSolver2D::FluidPlaneAddDensity(int x, int y, float amount)
{
    // int N = cube.size;
    // cube.density[IX(x, y, z)] += amount;
}

void FluidSolver2D::FluidPlaneAddVelocity(int x, int y, float amountX, float amountY)
{
    // int N = cube.size;
    // int index = IX(x, y, z);
    
    // cube.Vx[index] += amountX;
    // cube.Vy[index] += amountY;
    // cube.Vz[index] += amountZ;
}