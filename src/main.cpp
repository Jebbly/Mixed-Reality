#include <iostream>
#include "renderer.h"

const int width = 800;
const int height = 600;
int main(int argc, char* argv[])
{
    if(argc < 5)
    {
        std::cerr << "Usage: ./mixed_reality [vocabulary_file] [settings_file] [dataset_directory] [shader_directory]" << std::endl;
        return -1;
    }

    Renderer renderer(width, height, std::string(argv[4]));

    renderer.run();

    return 0;
}