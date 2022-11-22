#include <iostream>
#include "renderer.h"

const int width = 800;
const int height = 600;
int main()
{
    Renderer renderer(width, height);

    renderer.run();

    return 0;
}