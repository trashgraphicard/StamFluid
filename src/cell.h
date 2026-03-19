#ifndef CELL_H
#define CELL_H
// very lightweight cell class that holds raw data only.
struct Cell{
    float x;
    float y;
    float color[3];

    Cell(){
        x = 0.0f;
        y = 0.0f;
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
    }

    Cell(float px, float py, const float col[3]){
        x = px;
        y = py;
        color[0] = col[0];
        color[1] = col[1];
        color[2] = col[2];
    }

    void setColor(const float col[3]){
        color[0] = col[0];
        color[1] = col[1];
        color[2] = col[2];
    }
};

#endif // CELL_H