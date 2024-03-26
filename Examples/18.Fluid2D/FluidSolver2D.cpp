#include "FluidSolver2D.h"

#define IX(x, y) ((x) + (y) * N)

FluidSolver2D::FluidSolver2D(uint32_t size)
{
}

FluidSolver2D::~FluidSolver2D()
{
}

static void set_bnd(int b, std::vector<float>& x, int N)
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

static void lin_solve(int b, std::vector<float>& x, std::vector<float>& x0, float a, float c, int iter, int N)
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

static void diffuse (int b, std::vector<float>& x, std::vector<float>& x0, float diff, float dt, int iter, int N)
{
    float a = dt * diff * (N - 2) * (N - 2);
    lin_solve(b, x, x0, a, 1 + 6 * a, iter, N);
}

static void project(std::vector<float>& velocX, std::vector<float>& velocY, std::vector<float>& p, std::vector<float>& div, int iter, int N)
{
    for (int j = 1; j < N - 1; j++) {
        for (int i = 1; i < N - 1; i++) {
            div[IX(i, j)] = -0.5f*(
                     velocX[IX(i+1, j  )]
                    -velocX[IX(i-1, j  )]
                    +velocY[IX(i  , j+1)]
                    -velocY[IX(i  , j-1)]
                )/N;
            p[IX(i, j)] = 0;
        }
    }
    set_bnd(0, div, N); 
    set_bnd(0, p, N);
    lin_solve(0, p, div, 1, 6, iter, N);
    
    for (int j = 1; j < N - 1; j++) {
        for (int i = 1; i < N - 1; i++) {
            velocX[IX(i, j)] -= 0.5f * (  p[IX(i+1, j)]
                                         -p[IX(i-1, j)]) * N;
            velocY[IX(i, j)] -= 0.5f * (  p[IX(i, j+1)]
                                         -p[IX(i, j-1)]) * N;
        }
    }
    set_bnd(1, velocX, N);
    set_bnd(2, velocY, N);
}

void FluidSolver2D::FluidSolveStep(FluidPlane& cube)
{
    int N          = cube.size;
    float visc     = cube.visc;
    float diff     = cube.diff;
    float dt       = cube.dt;
    
    diffuse(1, cube.Vx0, cube.Vx, visc, dt, 4, N);
    diffuse(2, cube.Vy0, cube.Vy, visc, dt, 4, N);
    
    // project(Vx0, Vy0, Vz0, Vx, Vy, 4, N);
    
    // advect(1, Vx, Vx0, Vx0, Vy0, Vz0, dt, N);
    // advect(2, Vy, Vy0, Vx0, Vy0, Vz0, dt, N);
    // advect(3, Vz, Vz0, Vx0, Vy0, Vz0, dt, N);
    
    // project(Vx, Vy, Vz, Vx0, Vy0, 4, N);
    
    diffuse(0, cube.s, cube.density, diff, dt, 4, N);
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