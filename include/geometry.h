#ifndef GEOMETRY_H
#define GEOMETRY_H

// Attributes needed to draw a screen quad for the background image
const float quad_vertices[] = {
    -1.0f, -1.0f, 
    -1.0f,  1.0f, 
    1.0f,  1.0f, 
    1.0f, -1.0f
};

const float quad_texcoords[] = {
    0.0f, 0.0f, 
    0.0f, 1.0f, 
    1.0f, 1.0f, 
    1.0f, 0.0f
};

const int quad_indices[] = {
    0, 1, 2,
    0, 2, 3
};

// Attributes needed to draw a unit cube
const float cube_vertices[] = {
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f
};

const float cube_indices[] = {
    0, 1, 3, 
    3, 1, 2,
    1, 5, 2, 
    2, 5, 6,
    5, 4, 6, 
    6, 4, 7,
    4, 0, 7, 
    7, 0, 3,
    3, 2, 7, 
    7, 2, 6,
    4, 5, 0, 
    0, 5, 1
};

// const float cube_vertices[] = {-0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f};
// const int cube_indices[] = { 0, 1, 2 };


#endif // GEOMETRY_H