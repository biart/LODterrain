Dynamic LOD Terrain

Author: Artyom Bishev

This is a first attempt of making an LOD Terrain.

Available features in this release:

    Dynamic LOD terrain, loaded entirely in RAM and GPU memory. Extremely large terrain chunks still can't be processed, 
    but the current version can already be used in real-time applications such as games.

Just ready for release:

    Heightmap as texture stored in GPU and normalmap.
    Smooth transition between the nodes

In the nearest future:

    Dynamic processing of other landscape data, such as textures of rock and snow.
    Making extremely large terrain with dynamically uploading data from the hard disk.
    Parallelism

Libraries used:

    GLFW (http://www.glfw.org/)
    glm (http://glm.g-truc.net)

Project is now supported as Visual Studio solution
