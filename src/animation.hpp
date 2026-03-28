#ifndef ANIMATION_H
#define ANIMATION_H
#include "config.hpp"

struct Animation{
    std::string name;
    std::string pathDir;
    std::string pathDensAdd;
    std::string pathDensSub;
    std::string pathU;
    std::string pathV;

    std::vector<std::vector<float>> densAdd;
    std::vector<std::vector<float>> densSub;
    std::vector<std::vector<float>> u;
    std::vector<std::vector<float>> v;

    int resolution[2];
    int pixelPerFrame;
    int fps;
    int numFrames;
    int currentFrame;

    Animation(){
        name = "Bad Apple";
        resolution[0] = 96;
        resolution[1] = 72;
        pixelPerFrame = resolution[0] * resolution[1];
        fps = 30;
        //numFrames = 6571;
        numFrames = 6571;
        std::string resDir = std::to_string(resolution[0]) + "x" + std::to_string(resolution[1]);
        pathDir = "../badApple/" + resDir;
        pathDensAdd = pathDir + "/dens_add.bin";
        pathDensSub = pathDir + "/dens_sub.bin";
        pathU = pathDir + "/u.bin";
        pathV = pathDir + "/v.bin";

        densAdd.resize(numFrames, std::vector<float>(pixelPerFrame, 0.0f));
        densSub.resize(numFrames, std::vector<float>(pixelPerFrame, 0.0f));
        u.resize(numFrames, std::vector<float>(pixelPerFrame, 0.0f));
        v.resize(numFrames, std::vector<float>(pixelPerFrame, 0.0f));

        currentFrame = 0;
    }

    // not going to use this constructor anyway because I'm only running bad apple atm
    Animation(std::string n, int res[2], int fps){
        name = n;
        resolution[0] = res[0];
        resolution[1] = res[1];
        this->fps = fps;
    }

    void cacheData(){
        cacheAsType(densAdd, pathDensAdd, 1, 0.5f);
        cacheAsType(densSub, pathDensSub, 2, 1.0f);
        cacheAsType(u, pathU, 3, 0.7f);
        cacheAsType(v, pathV, 3, 0.7f);
    }

    void cacheAsType(std::vector<std::vector<float>>& x, std::string path, int type, float strength){
        // cache binary files into vectors, type determine type of caching
            // type 1: As adding density
            // type 2: As subtracting density
            // type 3: As zero centered velocity (Bin file contain normalized velocity, we need to remap it here so
            // it performs as intended in simulation)

        std::ifstream inFS(path, std::ios::binary);
        if (!inFS.is_open()){
            std::cout << "Failed to open file: " << path << std::endl;
            return;
        }

        std::vector<unsigned char> buffer(pixelPerFrame);
        int frameIdx = 0;

        while (frameIdx < numFrames && inFS.read(reinterpret_cast<char*>(buffer.data()), pixelPerFrame)){
            for (int i = 0; i < pixelPerFrame; i++){

                if(type == 1){
                    x[frameIdx][i] = (static_cast<float>(buffer[i]) / 255.0f) * strength;
                }
                else if(type == 2){
                    // negate
                    x[frameIdx][i] = -1.0f * (static_cast<float>(buffer[i])/255.0f) * strength;
                }
                else if(type == 3){
                    // remap to -1 ~ 1
                    x[frameIdx][i] = ((static_cast<float>(buffer[i])/255.0f)*2 - 1) * strength;
                }
            }
            frameIdx++;
        }
        if (!inFS.eof() && inFS.fail()){
            std::cout << "Warning: binary read failed before reaching EOF." << std::endl;
        }
        std::cout << "Loaded " << frameIdx << " frames from " << path << std::endl;
    }

    void nextFrame(){
        if(currentFrame < numFrames-1)
            currentFrame++;
    }

    std::vector<float>& getCurrentDensAdd(){
        return densAdd[currentFrame];
    }
    std::vector<float>& getCurrentDensSub(){
        return densSub[currentFrame];
    }
    std::vector<float>& getCurrentU(){
        return u[currentFrame];
    }
    std::vector<float>& getCurrentV(){
        return v[currentFrame];
    }
    
    void printInfo(){
        std::cout << name << std::endl;
        std::cout << numFrames << std::endl;
        std::cout << pathDir << std::endl;
    }
};


#endif // ANIMATION_H