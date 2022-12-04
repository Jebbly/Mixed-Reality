# Mixed Reality Rendering
This is my final course project for CS 445: Computational Photography. 

## Demo

## Organization

The code is organized as a standard C++ project, where ``/include`` has all the function and class declarations and ``/src`` has all the definitions. Since this project uses Git submodules, those are cloned into ``/extern``.

In ``light_estimation.h/cpp`` and ``depth_completion.h/cpp``, interfaces for light source estimation and depth completion are defined. Furthermore, these files also provide example implementations for both interfaces. Then ``renderer.h/cpp`` is the deferred renderer which incorporates the completed depth and estimated light when drawing objects.

The ``/config`` directory holds onto a YAML configuration for the camera model, and the ``/shaders`` directory stores all the GLSL shaders used for rendering.


## Cloning and Building
This project relies heavily upon the ORB-SLAM3 library for SLAM. However, ORB-SLAM3 depends on Pangolin, which conflicts with some OpenGL libraries used in this project. To address this, ORB-SLAM3 should be modified such that Pangolin is not required, and all related Pangolin/OpenGL calls in ORB-SLAM3 are commented out. Note that all examples in the ORB-SLAM3 CMake should thus also be disabled.

Once ORB-SLAM3 has been built, this project can be cloned and built with the following commands:
```
git clone --recursive https://github.com/Jebbly/Mixed-Reality.git
cd Mixed-Reality
mkdir build
cd build
cmake -DORB_SLAM3_DIR={PATH_TO_ORB_SLAM3} ..
make
```


## Running

```
./mixed_reality /home/jebbly/Desktop/Mixed-Reality/ORB-SLAM/Vocabulary/ORBvoc.txt /home/jebbly/Desktop/Mixed-Reality/Mixed-Reality/config/ETH3D_resize.yaml /home/jebbly/Desktop/Mixed-Reality/eth3d_table-3/ /home/jebbly/Desktop/Mixed-Reality/Mixed-Reality/shaders/
```

## Datasets
The current project uses a dataset from [ETH3D](https://www.eth3d.net/slam_datasets). More specifically, the video demo uses video from the "table_3" dataset.