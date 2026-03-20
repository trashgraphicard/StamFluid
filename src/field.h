#ifndef FIELD_H
#define FIELD_H
#include "config.h"
#include "cell.h"
#include "utils.h"

struct Field{
    // this class handles drawing process

    int dimension_ren[2];
    int numCells_ren; 

    float cellSizeX;
    float cellSizeY;

    std::vector<Cell> cells;

    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

    Field(){
        dimension_ren[0] = 0;
        dimension_ren[1] = 0;

        numCells_ren = 0;

        cellSizeX = 0;
        cellSizeY = 0;

        VAO = 0;
        VBO = 0;
        EBO = 0;
    }

    Field(const int dms[2]){
        dimension_ren[0] = dms[0];
        dimension_ren[1] = dms[1];

        numCells_ren = dimension_ren[0] * dimension_ren[1];

        // cell size here is relative to screen position
        cellSizeX = 2.0f / dimension_ren[0];
        cellSizeY = 2.0f / dimension_ren[1];

        _initialize_grid();
    }

    void _initialize_grid(){
        
        cells.reserve(numCells_ren);
        float tempColor[3] = {0.5f, 0.5f, 0.5f};
        for (int row = 0; row < dimension_ren[1]; row++){
            for (int col = 0; col < dimension_ren[0]; col++){
                float x = -1.0f + col*cellSizeX;
                float y = -1.0f + row*cellSizeY;
                cells.emplace_back(x, y, tempColor);
            }
        }
    }

    void initGL(){

        // define normalized, one unit sized grid, very easy to resize and offset later in shader
        float quadVertices[8] = {
            0.0f, 0.0f, // BL
            1.0f, 0.0f, // BR
            1.0f, 1.0f, // TR
            0.0f, 1.0f  // TL
        };
        unsigned int elementIndices[6] = {
           0, 1, 3,  // first triangle
           3, 1, 2   // second triangle
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // setup positions
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // setup element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elementIndices), elementIndices, GL_STATIC_DRAW);
        // unbind vertex array
        glBindVertexArray(0);
    }

    void draw(unsigned int shaderProgram) const {
        int offsetLocation = glGetUniformLocation(shaderProgram, "uOffset");
        int sizeLocation = glGetUniformLocation(shaderProgram, "uSize");
        int colorLocation = glGetUniformLocation(shaderProgram, "uColor");

        glBindVertexArray(VAO);

        for (const Cell& cell: cells){
            glUniform2f(offsetLocation, cell.x, cell.y);
            glUniform2f(sizeLocation, cellSizeX, cellSizeY);
            glUniform3f(colorLocation, cell.color[0], cell.color[1], cell.color[2]);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        // unbind vertex array
        glBindVertexArray(0);
    }

    void setCellColor(int x, int y, const float col[3]){
        cells[IX(dimension_ren[0], x, y)].setColor(col);
    }

    ~Field(){
        if(VBO) glDeleteBuffers(1, &VBO);
        if(EBO) glDeleteBuffers(1, &EBO);
        if(VAO) glDeleteVertexArrays(1, &VAO);
    }
};

#endif //FIELD_H