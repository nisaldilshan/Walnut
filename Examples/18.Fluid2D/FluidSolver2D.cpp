#include "FluidSolver2D.h"

#define IX(x, y) ((x) + (y) * N)

constexpr uint32_t linearSolveIterations = 4;

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

static void lin_solve(int b, std::vector<float>& x, std::vector<float>& x0, float a, float c, int N)
{
    float cRecip = 1.0 / c;
    for (int k = 0; k < linearSolveIterations; k++) {
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

FluidSolver2D::FluidSolver2D(FluidPlane& fluid)
    : m_fluid(fluid)
{
}

void FluidSolver2D::FluidSolveStep()
{
    int N          = m_fluid.size;
    float dt       = m_fluid.dt;
    
    Diffuse(1, m_fluid.Vx0, m_fluid.Vx, m_fluid.viscosity, dt);
    Diffuse(2, m_fluid.Vy0, m_fluid.Vy, m_fluid.viscosity, dt);
    
    Project(m_fluid.Vx0, m_fluid.Vy0, m_fluid.Vx, m_fluid.Vy);
    
    Advect(1, m_fluid.Vx, m_fluid.Vx0, m_fluid.Vx0, m_fluid.Vy0, dt);
    Advect(2, m_fluid.Vy, m_fluid.Vy0, m_fluid.Vx0, m_fluid.Vy0, dt);
    
    Project(m_fluid.Vx, m_fluid.Vy, m_fluid.Vx0, m_fluid.Vy0);
    
    Diffuse(0, m_fluid.density0, m_fluid.density, m_fluid.diffusion, dt);
    Advect(0, m_fluid.density, m_fluid.density0, m_fluid.Vx, m_fluid.Vy, dt);
}

void FluidSolver2D::FluidPlaneAddDensity(int x, int y, float amount)
{
    const int N = m_fluid.size;
    m_fluid.density[IX(x, y)] += amount;
}

void FluidSolver2D::FluidPlaneAddVelocity(int x, int y, float amountX, float amountY)
{
    const int N = m_fluid.size;
    int index = IX(x, y);
    
    m_fluid.Vx[index] += amountX;
    m_fluid.Vy[index] += amountY;
}

void FluidSolver2D::Advect(int b, std::vector<float> &d, std::vector<float> &d0, std::vector<float> &velocX, std::vector<float> &velocY, float dt)
{
    const int N = m_fluid.size;
    float i0, i1, j0, j1;
    
    float dtx = dt * (N - 2);
    float dty = dt * (N - 2);
    
    float s0, s1, t0, t1;
    float tmp1, tmp2, x, y;
    
    float Nfloat = N;
    float ifloat, jfloat;
    int i, j;
    

    for(j = 1, jfloat = 1; j < N - 1; j++, jfloat++) { 
        for(i = 1, ifloat = 1; i < N - 1; i++, ifloat++) {
            tmp1 = dtx * velocX[IX(i, j)];
            tmp2 = dty * velocY[IX(i, j)];
            x    = ifloat - tmp1; 
            y    = jfloat - tmp2;
            
            if(x < 0.5f) x = 0.5f; 
            if(x > Nfloat + 0.5f) x = Nfloat + 0.5f; 
            i0 = floorf(x); 
            i1 = i0 + 1.0f;
            if(y < 0.5f) y = 0.5f; 
            if(y > Nfloat + 0.5f) y = Nfloat + 0.5f; 
            j0 = floorf(y);
            j1 = j0 + 1.0f; 
            
            s1 = x - i0; 
            s0 = 1.0f - s1; 
            t1 = y - j0; 
            t0 = 1.0f - t1;
            
            int i0i = i0;
            int i1i = i1;
            int j0i = j0;
            int j1i = j1;
            
            d[IX(i, j)] = 
            
                s0 * ( t0 * d0[IX(i0i, j0i)]  +  t1 * d0[IX(i0i, j1i)])
              + s1 * ( t0 * d0[IX(i1i, j0i)]  +  t1 * d0[IX(i1i, j1i)]);
        }
    }
    set_bnd(b, d, N);
}

void FluidSolver2D::Diffuse(int b, std::vector<float>& x, std::vector<float>& x0, float diff, float dt)
{
    const int N = m_fluid.size;
    float a = dt * diff * (N - 2) * (N - 2);
    lin_solve(b, x, x0, a, 1 + 6 * a, N);
}

void FluidSolver2D::Project(std::vector<float>& velocX, std::vector<float>& velocY, std::vector<float>& p, std::vector<float>& div)
{
    const int N = m_fluid.size;
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
    lin_solve(0, p, div, 1, 6, N);
    
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
