#ifndef UTILS_H
#define UTILS_H
// utility functions for the project
#include "config.hpp"

unsigned int make_module(const std::string& filepath, unsigned int module_type){
    std::ifstream file;
    std::stringstream bufferedLines;
    std::string line;

    file.open(filepath);
    while(std::getline(file, line)){
        bufferedLines << line << "\n";
    }
    std::string shaderSource = bufferedLines.str();
    const char* shaderSrc = shaderSource.c_str();
    file.close();

    unsigned int shaderModule = glCreateShader(module_type);
    glShaderSource(shaderModule, 1, &shaderSrc, NULL);
    glCompileShader(shaderModule);

    int success;
    glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
    if(!success){
        char errorLog[1024];
        glGetShaderInfoLog(shaderModule, 1024, NULL, errorLog);
        std::cout << "Shader Module compilation error:\n" << errorLog << std::endl;
    }

    return shaderModule;
}

unsigned int make_shader(const std::string& vertex_filepath, const std::string fragment_filepath){
     std::vector<unsigned int> modules;
     modules.push_back(make_module(vertex_filepath, GL_VERTEX_SHADER));
     modules.push_back(make_module(fragment_filepath, GL_FRAGMENT_SHADER));

     unsigned int shader = glCreateProgram();
     for (unsigned int module: modules){
        glAttachShader(shader, module);
     }
     glLinkProgram(shader);

     int success;
     glGetProgramiv(shader, GL_LINK_STATUS, &success);
     if(!success){
        char errorLog[1024];
        glGetProgramInfoLog(shader, 1024, NULL, errorLog);
        std::cout << "Shader Linking error:\n" << errorLog << std::endl;
    }

    for (unsigned int module: modules){
        glDeleteShader(module);
     }
     
     return shader;
}

float randomFloat() {
    // function used for debug purpose
    // Cast to float/double before division to ensure floating-point arithmetic
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

int IX(int stride, int x, int y){
    // converts a 2 dimensional index to 1 dimensional index
    return y*stride + x;
}

float clamp(float x){
    return std::clamp(x, -1.0f, 1.0f);
}

#endif // UTILS_H