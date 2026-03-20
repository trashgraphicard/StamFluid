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
    std::vector<float> u_cur; 
    std::vector<float> v_cur;
    std::vector<float> u_prev;
    std::vector<float> v_prev;
    std::vector<float> u_src; 
    std::vector<float> v_src; 
    // density buffers
    std::vector<float> dens_cur;
    std::vector<float> dens_prev;
    std::vector<float> dens_src; 

    // divergence and pressure buffers
    std::vector<float> divergence;
    std::vector<float> pressure;

    Field field;
    float dt; // delta time
    float diffusion_coefficient;
    float viscosity;

    GLFWwindow* window;
    unsigned int shader;

    double prevMouseX = 0.0;
    double prevMouseY = 0.0;
    bool hasPrevMouse = false;

    FluidSim(const int dms[2], const int res[2]): field(dms){

        dimension_ren[0] = dms[0];
        dimension_ren[1] = dms[1];

        dimension_sim[0] = dms[0]+2;
        dimension_sim[1] = dms[1]+2;

        resolution[0] = res[0];
        resolution[1] = res[1];

        numCells_ren = dimension_ren[0] * dimension_ren[1];
        numCells_sim = dimension_sim[0] * dimension_sim[1];

        u_cur.resize(numCells_sim, 0.0f);
        v_cur.resize(numCells_sim, 0.0f);
        u_prev.resize(numCells_sim, 0.0f);
        v_prev.resize(numCells_sim, 0.0f);
        u_src.resize(numCells_sim, 0.0f);
        v_src.resize(numCells_sim, 0.0f);
        dens_cur.resize(numCells_sim, 0.0f);
        dens_prev.resize(numCells_sim, 0.0f);
        dens_src.resize(numCells_sim, 0.0f);
        divergence.resize(numCells_sim, 0.0f);
        pressure.resize(numCells_sim,0.0f);

        dt = 1e-2f;
        diffusion_coefficient = 1e-3;
        viscosity = 1e-3;

        shader = 0;

        _init_sim();
    }

    void _init_sim(){
        
        
        int n = 0;
        for (int row = 0; row < dimension_ren[1]; row++){
            for (int col = 0; col < dimension_ren[0]; col++){
                float grayscale = (float)n/numCells_ren;
                n++;
                u_cur[IX(dimension_sim[0], col+1, row+1)] = grayscale;
                dens_cur[IX(dimension_sim[0], col+1, row+1)] = grayscale;
                float tempColor[3] = {grayscale, grayscale, grayscale};
                field.setCellColor(col, row, tempColor);
            }
        }
        
        
        //int stride = dimension_sim[0];
        //int cx = dimension_ren[0] / 2;
        //int cy = dimension_ren[1] / 2;
        //dens_prev[IX(stride, cx, cy)] = 1.0f;
        //dens_prev[IX(stride, cx + 1, cy)] = 1.0f;
        //dens_prev[IX(stride, cx, cy + 1)] = 1.0f;
        //dens_prev[IX(stride, cx + 1, cy + 1)] = 1.0f;
    }

    void _add_source(std::vector<float>& x, std::vector<float>& s){
        // x: things to add to
        // s: the source to add to x
        for (int i=0; i<numCells_sim; i++){
            x[i] += dt*s[i];
        }
    }

    void _clear_sources() {
        std::fill(u_src.begin(), u_src.end(), 0.0f);
        std::fill(v_src.begin(), v_src.end(), 0.0f);
        std::fill(dens_src.begin(), dens_src.end(), 0.0f);
    }

    void _diffuse(int b, std::vector<float>& x, std::vector<float>&x0, float diff){

        // Diffuse value x with coefficient diff

        // b: bound type
        int rX = dimension_ren[0];
        int rY = dimension_ren[1];
        int stride = dimension_sim[0];

        float ax = dt*diff*rX*rX;
        float ay = dt*diff*rY*rY;

        // diffuse the value using the Gauss-Seidel diffusion equation
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

    void _advect(int b, std::vector<float>& d, std::vector<float>& d0, std::vector<float>& u, std::vector<float>& v){

        // Advect value d along velocity u and v

        int i, j; // position of the target cell
        int i0, j0, i1, j1; // positions of the 4 cells surrounding the source, which will be (i0, j0), (i1, j0), (i0, j1), and (i1, j1);
        float x, y; // position of the source velocity
        float s0, s1; // horizontal weighting for interpolation
        float t0, t1; // vertical weighting for inerpolation
        float dtx0, dty0; // x and y distance between target and source

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
                // these positions will align with the grid and hence are integers
                i0 = (int)x; i1 = i0+1;
                j0 = (int)y; j1 = j0+1;
                // interpolate the source density to get the target density
                s1 = x-i0; s0 = 1.0f-s1; // compute horizontal weight
                t1 = y-j0; t0 = 1.0f-t1; // compute vertical weight
                d[IX(stride, i, j)] = s0*(t0*d0[IX(stride, i0, j0)] + t1*d0[IX(stride, i0, j1)]) + 
                                      s1*(t0*d0[IX(stride, i1, j0)] + t1*d0[IX(stride, i1, j1)]);
            }
        }
        _set_bnd(b, d);
    }

    void _project(std::vector<float>& u, std::vector<float>&v, std::vector<float>& p, std::vector<float>& div){

        // compute the divergence free velocity field

        int i, j, k;
        int rX = dimension_ren[0];
        int rY = dimension_ren[1];
        int stride = dimension_sim[0];
        float hX = 1.0f / rX;
        float hY = 1.0f / rY;
        float aX = 1.0f / (hX*hX);
        float aY = 1.0f / (hY*hY);

        for(i=1; i<=rX; i++){
            for (j=1; j<=rY; j++) {
                div[IX(stride, i, j)] =
                    -0.5f * (
                    (u[IX(stride, i+1, j)] - u[IX(stride, i-1, j)]) / hX +
                    (v[IX(stride, i, j+1)] - v[IX(stride, i, j-1)]) / hY);
                // set all p to zero and solve it later using the Gauss-seidel diffusion equation
                p[IX(stride, i, j)] = 0.0f;
            }
        }

        _set_bnd(0, div);
        _set_bnd(0, p);

        for(k=0; k<20; k++){
            for(i=1; i<=rX; i++){
                for(j=1; j<=rY; j++){
                    p[IX(stride, i, j)] = (div[IX(stride, i, j)] + 
                                            aX*(p[IX(stride, i-1, j)] + p[IX(stride, i+1, j)]) + 
                                            aY*(p[IX(stride, i, j-1)] + p[IX(stride, i, j+1)]))
                                            / (2.0f*aX + 2.0f*aY);
                }
            }
            _set_bnd(0, p);
        }

        for(i=1; i<=rX; i++){
            for(j=1; j<=rY; j++){
                u[IX(stride, i, j)] -= 0.5f * (p[IX(stride, i+1, j)]-p[IX(stride, i-1, j)])/hX;
                v[IX(stride, i, j)] -= 0.5f * (p[IX(stride, i, j+1)]-p[IX(stride, i, j-1)])/hY;
            }
        }
        _set_bnd(1, u);
        _set_bnd(2, v);
    }

    void _density_step(){

        _add_source(dens_cur, dens_src);
        std::swap(dens_prev, dens_cur);
        _diffuse(0, dens_cur, dens_prev, diffusion_coefficient);
        std::swap(dens_prev, dens_cur);
        _advect(0, dens_cur, dens_prev, u_cur, v_cur);
    }

    void _velocity_step(){

        _add_source(u_cur, u_src);
        _add_source(v_cur, v_src);

        std::swap(u_prev, u_cur);
        _diffuse(1, u_cur, u_prev, viscosity);

        std::swap(v_prev, v_cur);
        _diffuse(2, v_cur, v_prev, viscosity);

        _project(u_cur, v_cur, pressure, divergence);

        std::swap(u_prev, u_cur);
        std::swap(v_prev, v_cur);

        _advect(1, u_cur, u_prev, u_prev, v_prev);
        _advect(2, v_cur, v_prev, u_prev, v_prev);

        _project(u_cur, v_cur, pressure, divergence);
    }

    void _set_bnd(int b, std::vector<float>& x){

        // Bound value x

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

    int _init_gl(){
        if (!glfwInit()){
            std::cout << "GLFW couldn't start" << std::endl;
            return -1;
        }
        window = glfwCreateWindow(resolution[0], resolution[1], "Fluid Sim go Brrrrr", NULL, NULL);

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

        shader = make_shader("../src/shaders/vertex.txt",
                            "../src/shaders/fragment.txt");
        
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        // very important, create necessary buffers for drawing after a valid GL context have been created
        field.initGL();

        return 0;
    }

    void _handle_source_input() {
        if (!window) return;
    
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
    
        bool leftDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)  == GLFW_PRESS;
        bool rightDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
        bool spaceDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    
        if (!leftDown && !rightDown) {
            hasPrevMouse = false;
            return;
        }
    
        int cellX = static_cast<int>((mouseX / resolution[0]) * dimension_ren[0]);
        int cellY = static_cast<int>((mouseY / resolution[1]) * dimension_ren[1]);
        cellY = dimension_ren[1] - 1 - cellY;
    
        if (cellX < 0) cellX = 0;
        if (cellX >= dimension_ren[0]) cellX = dimension_ren[0] - 1;
        if (cellY < 0) cellY = 0;
        if (cellY >= dimension_ren[1]) cellY = dimension_ren[1] - 1;
    
        int stride = dimension_sim[0];
        int simX = cellX + 1;
        int simY = cellY + 1;
    
        // left mouse: add density with a small brush
        if (leftDown) {
            for (int oy = -1; oy <= 1; oy++) {
                for (int ox = -1; ox <= 1; ox++) {
                    int sx = simX + ox;
                    int sy = simY + oy;
                    if (sx >= 1 && sx <= dimension_ren[0] &&
                        sy >= 1 && sy <= dimension_ren[1]) {
                        dens_src[IX(stride, sx, sy)] = 10.0f;
                    }
                }
            }
        }

        // space down: suck away density
        if (spaceDown) {
            for (int oy = -1; oy <= 1; oy++) {
                for (int ox = -1; ox <= 1; ox++) {
                    int sx = simX + ox;
                    int sy = simY + oy;
                    if (sx >= 1 && sx <= dimension_ren[0] &&
                        sy >= 1 && sy <= dimension_ren[1]) {
                        dens_src[IX(stride, sx, sy)] = -10.0f;
                    }
                }
            }
        }
    
        // right mouse: inject velocity based on mouse motion
        if (rightDown && hasPrevMouse) {
            float dx = static_cast<float>(mouseX - prevMouseX);
            float dy = static_cast<float>(prevMouseY - mouseY); // flip y
    
            for (int oy = -1; oy <= 1; oy++) {
                for (int ox = -1; ox <= 1; ox++) {
                    int sx = simX + ox;
                    int sy = simY + oy;
                    if (sx >= 1 && sx <= dimension_ren[0] &&
                        sy >= 1 && sy <= dimension_ren[1]) {
                        u_src[IX(stride, sx, sy)] = dx * 5.0f;
                        v_src[IX(stride, sx, sy)] = dy * 5.0f;
                    }
                }
            }
        }
    
        prevMouseX = mouseX;
        prevMouseY = mouseY;
        hasPrevMouse = true;
    }

    void _set_draw_type(int t){
        // t: type of data to visualize
        // 0: draw density
        // 1: maybe draw velocity, I will add that option later
        int rX = dimension_ren[0];
        int rY = dimension_ren[1];
        int stride = dimension_sim[0];
        if(t == 0){
            for (int i=0; i<rX; i++){
                for(int j=0; j<rY; j++){
                    float dens = dens_cur[IX(stride, i+1, j+1)];
                    float color[3] = {dens, dens, dens};
                    field.setCellColor(i, j, color);
                }
            }
        } else {
            throw std::invalid_argument("Bruh read the comment bruh");
        }
    }

    int run(){

        _init_gl();

        double lastTime = glfwGetTime();
        int frameCount = 0;

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(shader);

            _handle_source_input();
            _velocity_step();
            _density_step();
            _set_draw_type(0);
            _clear_sources();
            field.draw(shader);

            glfwSwapBuffers(window);

            frameCount++;
            double currentTime = glfwGetTime();
            if (currentTime - lastTime >= 1.0) {
                std::cout << "FPS: " << frameCount << std::endl;
                frameCount = 0;
                lastTime = currentTime;
            }
        }
        glDeleteProgram(shader);
        return 0;
    }

    ~FluidSim(){
        glfwTerminate();
    }
};

#endif //FLUID_SIM_H