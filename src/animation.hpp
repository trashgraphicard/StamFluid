#ifndef ANIMATION_H
#define ANIMATION_H
#include "config.hpp"

struct Animation{
    std::string name;
    std::string pathDir;
    std::string pathFramesTxt;
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
        numFrames = 500;
        std::string resDir = std::to_string(resolution[0]) + "x" + std::to_string(resolution[1]);
        pathDir = "../badApple/" + resDir;
        pathFramesTxt = pathDir + "/frames.txt";
        currentFrame = 0;
    }

    // not going to use this constructor anyway because I'm only running bad apple atm
    Animation(std::string n, int res[2], int fps){
        name = n;
        resolution[0] = res[0];
        resolution[1] = res[1];
        this->fps = fps;
    }

    void cacheFrames(){
        frames.clear();
        frames.resize(numFrames, std::vector<float>(pixelPerFrame, 0.0f));

        std::ifstream inFS(pathFramesTxt);
        if (!inFS.is_open()){
            std::cout << "Failed to open frame data file: " << pathFramesTxt << std::endl;
            return;
        }

        std::string line;
        int frameIdx = 0;

        while (std::getline(inFS, line) && frameIdx < numFrames){
            std::istringstream iss(line);

            for (int i = 0; i < pixelPerFrame; i++){
                float value;
                if (!(iss >> value)){
                    std::cout << "Warning: frame " << frameIdx
                              << " has fewer than " << pixelPerFrame
                              << " values." << std::endl;
                    break;
                }

                // if file stores 0..255 grayscale, normalize:
                frames[frameIdx][i] = value / 255.0f;

                // if file stores 0 or 1 already, use this instead:
                // frames[frameIdx][i] = value;
            }

            std::cout << "Caching frame: " << frameIdx << std::endl;
            frameIdx++;
        }

        if (frameIdx != numFrames){
            std::cout << "Warning: expected " << numFrames
                      << " frames, loaded " << frameIdx << std::endl;
        } else {
            std::cout << "Loaded " << frameIdx << " frames from " << pathFramesTxt << std::endl;
        }
    }

    void nextFrame(){
        if(currentFrame < numFrames-1)
            currentFrame++;
    }

    std::vector<float>& getCurrentFrame(){
        return frames[currentFrame];
    }

    void printInfo(){
        std::cout << name << std::endl;
        std::cout << numFrames << std::endl;
        std::cout << pathDir << std::endl;
        std::cout << pathFramesTxt << std::endl;
    }
};


#endif // ANIMATION_H