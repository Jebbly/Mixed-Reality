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
    0, 2, 1,
    3, 2, 0
};

// Attributes needed to draw a cube object
// Each vertex position is drawn three times
// corresponding to three different vertex normals
const float cube_vertices[] = {
    // Bottom
    -0.05f,  0.0f, -0.05f,
    -0.05f,  0.0f,  0.05f,
     0.05f,  0.0f, -0.05f,
     0.05f,  0.0f,  0.05f,
     
    // Front
     0.05f,  0.0f, -0.05f,
     0.05f, -0.1f, -0.05f,
    -0.05f,  0.0f, -0.05f,
    -0.05f, -0.1f, -0.05f,

    // Right
     0.05f,  0.0f,  0.05f,
     0.05f, -0.1f,  0.05f,
     0.05f,  0.0f, -0.05f,
     0.05f, -0.1f, -0.05f,

    // Back
    -0.05f,  0.0f,  0.05f,
    -0.05f, -0.1f,  0.05f,
     0.05f,  0.0f,  0.05f,
     0.05f, -0.1f,  0.05f,

    // Left
    -0.05f,  0.0f,  0.05f,
    -0.05f,  0.0f, -0.05f,
    -0.05f, -0.1f,  0.05f,
    -0.05f, -0.1f, -0.05f,

    // Top
     0.05f, -0.1f, -0.05f,
     0.05f, -0.1f,  0.05f,
    -0.05f, -0.1f, -0.05f,
    -0.05f, -0.1f,  0.05f,
};

const float cube_normals[] = {
    // Bottom
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,
     0.0f,  1.0f,  0.0f,

    // Front
     0.0f,  0.0f, -1.0f,
     0.0f,  0.0f, -1.0f,
     0.0f,  0.0f, -1.0f,
     0.0f,  0.0f, -1.0f,

    // Right
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,
     1.0f,  0.0f,  0.0f,

    // Back
     0.0f,  0.0f,  1.0f,
     0.0f,  0.0f,  1.0f,
     0.0f,  0.0f,  1.0f,
     0.0f,  0.0f,  1.0f,

    // Left
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f,

    // Top
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,
};

const int cube_indices[] = { 
    // Bottom
     0,  1,  2,
     3,  2,  1,

    // Front
     4,  5,  6,
     7,  6,  5,

    // Right
     8,  9, 10,
    11, 10,  9,

    // Back
    12, 13, 14,
    15, 14, 13,

    // Left
    16, 17, 18,
    19, 18, 17,

    // Top
    20, 21, 22,
    23, 22, 21
};

#endif // GEOMETRY_H