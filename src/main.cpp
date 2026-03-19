#include "config.h"
#include "utils.h"
#include "field.h"
#include "fluid_sim.h"

int main(){
    /*
    GLFWwindow* window;

    if (!glfwInit()){
        std::cout << "GLFW couldn't start" << std::endl;
        return -1;
    }
    window = glfwCreateWindow(1280, 970, "4:3 test window", NULL, NULL);
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

    int dimension[2] = {72, 54};
    Field f = Field(dimension);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader);
        
        for (int row = 0; row < dimension[1]; row++){
            for (int col = 0; col < dimension[0]; col++){
                //float nC[3] = {randomFloat(), randomFloat(), randomFloat()};
                //f.setCellColor(col, row, nC);
                if (row == 7){
                    float color[3] = {1.0f, 0.0f, 0.0f};
                    f.setCellColor(col, row, color);
                }
            }
        }
        
        f.draw(shader);

        glfwSwapBuffers(window);
    }

    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
    */

   int dimension[2] = {36, 27};
   int resolution[2] = {720, 540};
   FluidSim sim = FluidSim(dimension, resolution);
   sim.run();
}