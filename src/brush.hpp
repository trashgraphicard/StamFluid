#ifndef BRUSH_H
#define BRUSH_H
#include "config.hpp"

struct Brush{
    std::string name;
    int resolution[2];
    int numPixels;

    std::string pathDir;
    std::string pathDensAdd;
    std::string pathDensSub;
    std::string pathU;
    std::string pathV;

    std::vector<float> densAddBase;
    std::vector<float> densSubBase;
    std::vector<float> uBase;
    std::vector<float> vBase;

    std::vector<float> densAdd;
    std::vector<float> densSub;
    std::vector<float> u;
    std::vector<float> v;

    double prevMouseX = 0.0;
    double prevMouseY = 0.0;
    bool hasPrevMouse = false;

    float offsetX = 0.0f;
    float offsetY = 0.0f;

    Brush(std::string n, int res[2]){
        name = n;
        resolution[0] = res[0];
        resolution[1] = res[1];
        numPixels = resolution[0]*resolution[1];
        std::string resDir = std::to_string(resolution[0]) + "x" + std::to_string(resolution[1]);
        pathDir = "../brush/" + name + '/' + resDir;
        pathDensAdd = pathDir + "/dens_add.bin";
        pathDensSub = pathDir + "/dens_sub.bin";
        pathU = pathDir + "/u.bin";
        pathV = pathDir + "/v.bin";

        densAddBase.resize(numPixels, 0.0f);
        densSubBase.resize(numPixels, 0.0f);
        uBase.resize(numPixels, 0.0f);
        vBase.resize(numPixels, 0.0f);

        densAdd.resize(numPixels, 0.0f);
        densSub.resize(numPixels, 0.0f);
        u.resize(numPixels, 0.0f);
        v.resize(numPixels, 0.0f);
    }

    void cacheData(){
        cacheAsType(densAdd, pathDensAdd, 1, 0.08f);
        cacheAsType(densSub, pathDensSub, 2, 1.0f);
        cacheAsType(u, pathU, 3, 0.1f);
        cacheAsType(v, pathV, 3, 0.1f);

        densAddBase = densAdd;
        densSubBase = densSub;
        uBase = u;
        vBase = v;
    }

    void cacheAsType(std::vector<float>& x, std::string path, int type, float strength){
        // similar to how the method in Animation class works
        std::ifstream inFS(path, std::ios::binary);
        if (!inFS.is_open()){
            std::cout << "Failed to open file: " << path << std::endl;
            return;
        }

        std::vector<unsigned char> buffer(numPixels);

        if (!inFS.read(reinterpret_cast<char*>(buffer.data()), numPixels)) {
            std::cout << "Failed to read full brush file: " << path << std::endl;
        return;
        }
        for(int i=0; i<numPixels; i++){
            if(type == 1){
                x[i] = (buffer[i]/255.0f) * strength;
            }
            else if(type == 2){
                x[i] = -1.0f * (buffer[i]/255.0f) * strength;
            }
            else if(type == 3){
                x[i] = ((buffer[i]/255.0f)*2-1) * strength;
            }
        }
    }

    std::vector<float>& getDensAdd(){
        return densAdd;
    }
    std::vector<float>& getDensSub(){
        return densSub;
    }
    std::vector<float>& getU(){
        return u;
    }
    std::vector<float>& getV(){
        return v;
    }

    void _clearCurrentBrush(){
        std::fill(densAdd.begin(), densAdd.end(), 0.0f);
        std::fill(densSub.begin(), densSub.end(), 0.0f);
        std::fill(u.begin(), u.end(), 0.0f);
        std::fill(v.begin(), v.end(), 0.0f);
    }

    void _rebuildShifedBrush(){
        _clearCurrentBrush();

        int shiftX = static_cast<int>(std::round(offsetX));
        int shiftY = static_cast<int>(std::round(offsetY));
        std::cout << "Brush moved " << shiftX << " X grids from the original position\n";
        std::cout << "Brush moved " << shiftY << " Y grids from the original postition\n";
        int stride = resolution[0];

        for(int y=0; y<resolution[1]; y++){
            for(int x=0; x<resolution[0]; x++){
                // int srcIdx = y * resolution[0] + x; // flattened 1d index
                int srcIdx = IX(stride, x, y);

                int dstX = x + shiftX;
                int dstY = y + shiftY;
                // check if moved out of bound
                if(dstX<0 || dstX>=resolution[0] || dstY<0 || dstY>=resolution[1]){
                    continue;
                }
                
                int dstIdx = IX(stride, dstX, dstY);

                densAdd[dstIdx] = densAddBase[srcIdx];
                densSub[dstIdx] = densSubBase[srcIdx];
                u[dstIdx] = uBase[srcIdx];
                v[dstIdx] = vBase[srcIdx];

            }
        }
    }

    void moveBrush(GLFWwindow* window){
        if (!window){
            std::cout << "No GL Window context\n";
            return;
        }

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) != GLFW_PRESS){
            hasPrevMouse = false;
            return;
        }

        if(!hasPrevMouse){
            prevMouseX = mouseX;
            prevMouseY = mouseY;
            hasPrevMouse = true;
            return;
        }

        int winW, winH;
        glfwGetWindowSize(window, &winW, &winH);
        
        double dxPixels = mouseX - prevMouseX;
        double dyPixels = mouseY - prevMouseY;
        //std::cout << "Mouse moved by " << dxPixels << " X pixels\n";
        //std::cout << "Mouse moved by " << dyPixels << " Y pixels\n";

        // convert window-pixel motion to brush-grid motion
        // flip y because opengl is weird
        offsetX += static_cast<float>(dxPixels * resolution[0] / static_cast<double>(winW));
        offsetY += static_cast<float>(dyPixels * resolution[1] / static_cast<double>(winH));

        _rebuildShifedBrush();

        prevMouseX = mouseX;
        prevMouseY = mouseY;
    }

    void printInfo(){
        std::cout << "Path: " << pathDir << "\n";
        std::cout << "Path: " << pathDensAdd << "\n";
        std::cout << "Path: " << pathDensSub << "\n";
        std::cout << "Path: " << pathU << "\n";
        std::cout << "Path: " << pathV << "\n";
        std::cout << "Pixels: " << numPixels << "\n";
    }
};

#endif // BRUSH_H