#ifndef CELL_MESH_H
#define CELL_MESH_H
// lefacy class that handles drawing individual cells
// This is scraped due to performance issues
// But I will leave the code here anyway because it is my blood and tears
#include "config.h"

struct CellMesh{
    float position[2];
    float size;
    float positions[8];
    float color[3];
    int elementIndices[6];

    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    CellMesh(const float pos[2], float s, const float col[3]){
        size = s;

        position[0] = pos[0];
        position[1] = pos[1];

        color[0] = col[0];
        color[1] = col[1];
        color[2] = col[2];

        _calibrate_positions();
        _calibrate_elements();
        _setup_mesh();

    }

    void _calibrate_positions(){
        float x = position[0];
        float y = position[1];

        // BL -> TL -> TR -> BR

        positions[0] = x;
        positions[1] = y;

        positions[2] = x + size;
        positions[3] = y;

        positions[4] = x + size;
        positions[5] = y + size;

        positions[6] = x;
        positions[7] = y + size;
    }

    void _calibrate_elements(){
        // frist triangle
        elementIndices[0] = 0;
        elementIndices[1] = 1;
        elementIndices[2] = 3;
        // second triangle
        elementIndices[3] = 3;
        elementIndices[4] = 1;
        elementIndices[5] = 2;
    }

    void _setup_mesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // setup positions
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // setup element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementIndices), elementIndices, GL_STATIC_DRAW);

        // unbind the vertex array
        glBindVertexArray(0);
    }

    void updateColor(const float newColor[3]){
        color[0] = newColor[0];
        color[1] = newColor[1];
        color[2] = newColor[2];
    }

    void draw(unsigned int shaderProgram) const{

        // set the uniform color of the cell
        int colorLocation = glGetUniformLocation(shaderProgram, "uColor");
        glUniform3f(colorLocation, color[0], color[1], color[2]);

        // before drawing the shape, bind the vertex array first
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // then unbind it post draw
        glBindVertexArray(0);
    }

    ~CellMesh(){
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);
    }
};

#endif // CELL_MESH_H