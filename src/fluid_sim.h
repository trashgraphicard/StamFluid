#ifndef FLUID_SIM_H
#define FLUID_SIM_H
#include "config.h"
#include "field.h"
#include "utils.h"

struct FluidSim{

    // simulation needs a layer of boundry cell, so the size between sim stage and rendering stage would be different
    // separate into sim and rendering for my own sanity
    int dimension_ren[2]; // number of cells x and y
    int dimension_sim[2];
    int numCells_ren; // total number of cells
    int numCells_sim;

    int resolution[2]; // Window resolution

    // u and v are horizontal and vertical component of velocity vector
    std::vector<float> u; 
    std::vector<float> v;
    std::vector<float> u_prev;
    std::vector<float> v_prev;
    // dens 
    std::vector<float> dens;
    std::vector<float> dens_prev;

    Field field;
    float dt; // delta time
    float diff; // diffusion coefficient

    FluidSim(const int dms[2], const int res[2]): field(dms){

        dimension_ren[0] = dms[0];
        dimension_ren[1] = dms[1];

        dimension_sim[0] = dms[0]+2;
        dimension_sim[1] = dms[1]+2;

        resolution[0] = res[0];
        resolution[1] = res[1];

        numCells_ren = dimension_ren[0] * dimension_ren[1];
        numCells_sim = dimension_sim[0] * dimension_sim[1];

        u.resize(numCells_sim, 0.0f);
        v.resize(numCells_sim, 0.0f);
        u_prev.resize(numCells_sim, 0.0f);
        v_prev.resize(numCells_sim, 0.0f);
        dens.resize(numCells_sim, 0.0f);
        dens_prev.resize(numCells_sim, 0.0f);

        dt = 1e-3;
        diff = 1e-7;

        _init_sim();
    }

    void _init_sim(){
        std::cout << "Initializing sim" << std::endl;
        int n = 0;
        for (int row = 0; row < dimension_ren[1]; row++){
            for (int col = 0; col < dimension_ren[0]; col++){
                float grayscale = (float)n/numCells_ren;
                n++;
                u[IX(dimension_sim[0], col, row)] = grayscale;
                float tempColor[3] = {grayscale, grayscale, grayscale};
                field.setCellColor(col, row, tempColor);
            }
        }
    }

    void _diffuse(int b, std::vector<float>& x, std::vector<float>&x0){
        // taking notes as I go
        // b: bound type
        // diff: diffusion coefficient (how fast things spreads)
        // dt: delta time (smaller = more stable, and slower spread; faster = faster motion but can be unstable)
        int rX = dimension_ren[0];
        int rY = dimension_ren[1];
        int stride = dimension_sim[0];
        //float a = dt*diff*rX*rX;
        float ax = dt*diff*rX*rX;
        float ay = dt*diff*rY*rY;

        for(int k=0; k<20; k++){
            for(int i=1; i <= rX; i++){
                for (int j=1; j<=rY; j++) {
                    x[IX(stride, i, j)] = (x0[IX(stride, i, j)] + 
                                        ax*(x[IX(stride, i-1, j)] + x[IX(stride, i+1, j)]) +
                                        ay*(x[IX(stride, i, j-1)] + x[IX(stride, i, j+1)])) / 
                                        (1 + 2*ax + 2*ay);
                }
            }
            _set_bnd(b, x);
        }
    }

    void _advect(int b, std::vector<float>& d, std::vector<float>& d0){
        int i, j, i0, j0, i1, j1;
        float x, y, s0, t0, s1, t1, dtx0, dty0;

        int rX = dimension_ren[0];
        int rY = dimension_ren[1];
        int stride = dimension_sim[0];
        dtx0 = dt*rX;
        dty0 = dt*rY;

        for (i=1; i<=rX; i++){
            for (j=1; j<=rY; j++){
                // trace back from the current cell along the velocity to the source position
                x = i-dtx0*u[IX(stride, i, j)];
                y = j-dty0*v[IX(stride, i, j)];
                // clamp the source position
                if (x<0.5) x = 0.5;
                if (x>rX+0.5) x = rX+0.5;
                if (y<0.5) y = 0.5;
                if (y>rY+0.5) y = rY+0.5;
                // find the position of the 4 cells surrounding the source
                i0 = (int)x; i1 = i0+1;
                j0 = (int)y; j1 = j0+1;
                // interpolate the density
                s1 = x-i0; s0 = 1.0f-s1;
                t1 = y-j0; t0 = 1.0f-t1;
                d[IX(stride, i, j)] = s0*(t0*d0[IX(stride, i0, j0)] + t1*d0[IX(stride, i0, j1)]) + 
                                      s1*(t0*d0[IX(stride, i1, j0)] + t1*d0[IX(stride, i1, j1)]);
            }
        }
        _set_bnd(b, d);
    }

    void _density_step(){
        std::swap(dens_prev, dens);
        _diffuse(0, dens, dens_prev);
        std::swap(dens_prev, dens);
        _advect(0, dens, dens_prev);
    }

    void _set_bnd(int b, std::vector<float>& x){
        // b: type of bound.
        //  0 -> continuity
        //  1 -> invert velocity x (when collide with vertical border)
        //  2 -> invert velocity y (when collide with horizontal border)
        int rX = dimension_ren[0];
        int rY = dimension_ren[1];
        int stride = dimension_sim[0]; // full length of a row, use for reading from arrays using IX function
        // solve vertical bound
        for (int i = 1; i <= rY; i++){
            x[IX(stride, 0   , i)] = b==1 ? -x[IX(stride, 1 , i)] : x[IX(stride, 1 , i)];
            x[IX(stride, rX+1, i)] = b==1 ? -x[IX(stride, rX, i)] : x[IX(stride, rX, i)];
        }
        // solve horizontal bound
        for (int i = 1; i <= rX; i++){
            x[IX(stride, i,    0)] = b==2 ? -x[IX(stride, i,  1)] : x[IX(stride, i,  1)];
            x[IX(stride, i, rY+1)] = b==2 ? -x[IX(stride, i, rY)] : x[IX(stride, i, rY)];
        }
        // solve corners
        x[IX(stride, 0,       0)] = 0.5*(x[IX(stride, 1,     0)] + x[IX(stride, 0,     1)]); //BL
        x[IX(stride, 0,    rY+1)] = 0.5*(x[IX(stride, 1,  rY+1)] + x[IX(stride, 0,    rY)]); //TL
        x[IX(stride, rX+1,    0)] = 0.5*(x[IX(stride, rX,    0)] + x[IX(stride, rX+1,  1)]); //BR
        x[IX(stride, rX+1, rY+1)] = 0.5*(x[IX(stride, rX, rY+1)] + x[IX(stride, rX+1, rY)]); //TR
    }

    int run(){
        GLFWwindow* window;

        std:: cout << "starting..." << std::endl;

        if (!glfwInit()){
            std::cout << "GLFW couldn't start" << std::endl;
            return -1;
        }
        window = glfwCreateWindow(resolution[0], resolution[1], "4:3 test window", NULL, NULL);

        if (!window) {
            std::cout << "Window creation failed" << std::endl;
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
            glfwTerminate();
            return -1;
        }

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        unsigned int shader = make_shader("../src/shaders/vertex.txt",
                                        "../src/shaders/fragment.txt");

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        // very important, create necessary buffers for drawing after a valid GL context have been created
        field.initGL();

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(shader);
        
            field.draw(shader);

            glfwSwapBuffers(window);
        }
        glDeleteProgram(shader);
        return 0;
    }

    ~FluidSim(){
        glfwTerminate();
    }
};

#endif //FLUID_SIM_H