# Mixed Reality Rendering
This project started as a final course project for CS 445: Computational Photography, and is a toy example of how depth completion and light source estimation could be used in a mixed reality rendering pipeline. It extends upon [this ORB-SLAM AR demo](https://github.com/raulmur/ORB_SLAM2/tree/master/Examples/ROS/ORB_SLAM2/src/AR). The following video uses the "table_3" dataset from [ETH3D](https://www.eth3d.net/slam_datasets) with dense depth images that were computed offline and randomized light sources:

https://user-images.githubusercontent.com/38666957/208049048-01d2e3a8-479c-425d-b820-ad0ff8d91871.mp4

In ``light_estimation.*`` and ``depth_completion.*``, interfaces for light source estimation and depth completion are defined. These files also provide example implementations for both interfaces. Then ``renderer.*`` is the deferred renderer which incorporates the completed depth and estimated light when drawing objects. The ``/config`` directory holds onto a YAML configuration for the camera model, and the ``/shaders`` directory stores all the GLSL shaders used for rendering.

## Building and Running
This project relies heavily upon the ORB-SLAM3 library for SLAM. However, ORB-SLAM3 depends on Pangolin, which conflicts with some OpenGL libraries used in this project. To address this, ORB-SLAM3 should be modified such that Pangolin is not required, and all related Pangolin/OpenGL calls in ORB-SLAM3 should be commented out. Note that all examples in the ORB-SLAM3 CMake should thus also be disabled. Once ORB-SLAM3 has been built, this project can be cloned and built with the following commands:
```
git clone --recursive https://github.com/Jebbly/Mixed-Reality.git
cd Mixed-Reality
mkdir build
cd build
cmake -DORB_SLAM3_DIR={PATH_TO_ORB_SLAM3} ..
make
```

Assuming that everything has been built properly, ``main.cpp`` provides instructions with how to run the demo. An example command would be the following:

```
./mixed_reality /home/jebbly/Desktop/Mixed-Reality/ORB-SLAM/Vocabulary/ORBvoc.txt /home/jebbly/Desktop/Mixed-Reality/Mixed-Reality/config/ETH3D_resize.yaml /home/jebbly/Desktop/Mixed-Reality/eth3d_table-3/ /home/jebbly/Desktop/Mixed-Reality/Mixed-Reality/shaders/ /home/jebbly/Desktop/Mixed-Reality/model.gltf
```

## To-Do

This project likely needs some modifications for an easier setup process. I might also play around with my own implementations for live cameras, SLAM, and learning-models for depth completion and light source estimation. Otherwise, most of this project will be continued as work with [ILLIXR](https://github.com/ILLIXR/ILLIXR). 
