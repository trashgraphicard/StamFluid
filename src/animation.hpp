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
    std::string pathFramesBin;

    std::vector<std::vector<float>> densAdd;
    std::vector<std::vector<float>> densSub;
    std::vector<std::vector<float>> u;
    std::vector<std::vector<float>> v;
    std::vector<std::vector<float>> frames;

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
        numFrames = 1000;
        std::string resDir = std::to_string(resolution[0]) + "x" + std::to_string(resolution[1]);
        pathDir = "../badApple/" + resDir;
        pathDensAdd = pathDir + "/dens_add.bin";
        pathDensSub = pathDir + "/dens_sub.bin";
        pathU = pathDir + "/u.bin";
        pathV = pathDir + "/v.bin";
        pathFramesBin = pathDir + "/frames.bin";

        densAdd.resize(numFrames, std::vector<float>(pixelPerFrame, 0.0f));
        densSub.resize(numFrames, std::vector<float>(pixelPerFrame, 0.0f));
        u.resize(numFrames, std::vector<float>(pixelPerFrame, 0.0f));
        v.resize(numFrames, std::vector<float>(pixelPerFrame, 0.0f));
        frames.resize(numFrames, std::vector<float>(pixelPerFrame, 0.0f));

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
        cacheAsType(u, pathU, 3, 2.0f);
        cacheAsType(v, pathV, 3, 2.0f);
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

    // read the binary files and cache them in vector
    void cacheFrames(){

        std::ifstream inFS(pathDensAdd, std::ios::binary);
        if (!inFS.is_open()){
            std::cout << "Failed to open frame data file: " << pathDensAdd << std::endl;
            return;
        }

        std::vector<unsigned char> buffer(pixelPerFrame);
        int frameIdx = 0;

        while (frameIdx < numFrames &&
               inFS.read(reinterpret_cast<char*>(buffer.data()), pixelPerFrame)){

            for (int i = 0; i < pixelPerFrame; i++){
                frames[frameIdx][i] = (buffer[i] / 255.0f);
            }

            std::cout << "Caching frame: " << frameIdx << std::endl;
            frameIdx++;
        }

        if (!inFS.eof() && inFS.fail()){
            std::cout << "Warning: binary read failed before reaching EOF." << std::endl;
        }

        if (frameIdx != numFrames){
            std::cout << "Warning: expected " << numFrames
                      << " frames, loaded " << frameIdx << std::endl;
        } else {
            std::cout << "Loaded " << frameIdx << " frames from " << pathDensAdd << std::endl;
        }
    }

    /*
    void cacheOpticalFlow(){
        int numFlowFrames = numFrames - 1;

        flowX.clear();
        flowY.clear();
        flowX.resize(numFlowFrames, std::vector<float>(pixelPerFrame, 0.0f));
        flowY.resize(numFlowFrames, std::vector<float>(pixelPerFrame, 0.0f));

        std::ifstream inFSX(pathFlowXBin, std::ios::binary);
        if (!inFSX.is_open()){
            std::cout << "Failed to open optical flow X file: " << pathFlowXBin << std::endl;
            return;
        }

        std::ifstream inFSY(pathFlowYBin, std::ios::binary);
        if (!inFSY.is_open()){
            std::cout << "Failed to open optical flow Y file: " << pathFlowYBin << std::endl;
            return;
        }

        int frameIdx = 0;

        while (frameIdx < numFlowFrames &&
               inFSX.read(reinterpret_cast<char*>(flowX[frameIdx].data()),
                          pixelPerFrame * sizeof(float)) &&
               inFSY.read(reinterpret_cast<char*>(flowY[frameIdx].data()),
                          pixelPerFrame * sizeof(float))) {

            std::cout << "Caching optical flow frame: " << frameIdx << std::endl;
            frameIdx++;
        }

        if ((!inFSX.eof() && inFSX.fail()) || (!inFSY.eof() && inFSY.fail())){
            std::cout << "Warning: binary optical flow read failed before reaching EOF." << std::endl;
        }

        if (frameIdx != numFlowFrames){
            std::cout << "Warning: expected " << numFlowFrames
                      << " optical flow frames, loaded " << frameIdx << std::endl;
        } else {
            std::cout << "Loaded " << frameIdx << " optical flow frames from "
                      << pathFlowXBin << " and " << pathFlowYBin << std::endl;
        }
    }

    void cacheSdfAsDens(){
        sdfDens.clear();
        sdfDens.resize(numFrames, std::vector<float>(pixelPerFrame, 0.0f));

        std::ifstream inFS(pathSdfBin, std::ios::binary);
        if (!inFS.is_open()){
            std::cout << "Failed to open sdf file: " << pathSdfBin <<   std::endl;
            return;
        }

        std::vector<float> buffer(pixelPerFrame, 0.0f);
        int frameIdx = 0;

        while (frameIdx < numFrames &&
               inFS.read(reinterpret_cast<char*>(buffer.data()),
                         pixelPerFrame * sizeof(float))) {

            for (int i = 0; i < pixelPerFrame; i++){
                float v = buffer[i];

                // clamp to [0, 1]
                if (v < 0.0f) v = 0.0f;
                if (v > 1.0f) v = 1.0f;

                // invert: 1 - v
                v = 1.0f - v;

                // expand to [-1, 1]
                v = v * 2.0f - 0.5f;
                // shrink range
                v *= 0.1f;

                sdfDens[frameIdx][i] = v;
            }

            std::cout << "Caching sdf dens frame: " << frameIdx << std::endl;
            frameIdx++;
        }

        if (!inFS.eof() && inFS.fail()){
            std::cout << "Warning: binary sdf read failed before reaching EOF.  " << std::endl;
        }

        if (frameIdx != numFrames){
            std::cout << "Warning: expected " << numFrames
                      << " sdf frames, loaded " << frameIdx << std::endl;
        } else {
            std::cout << "Loaded " << frameIdx
                      << " sdf dens frames from " << pathSdfBin << std::endl;
        }
    }
    */

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
    std::vector<float>& getCurrentFrame(){
        return frames[currentFrame];
    }

    void printInfo(){
        std::cout << name << std::endl;
        std::cout << numFrames << std::endl;
        std::cout << pathDir << std::endl;
    }
};


#endif // ANIMATION_H