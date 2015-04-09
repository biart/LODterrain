/*
	Scene class
	Defines 3D-scene
*/

#ifndef SCENE_H
#define SCENE_H

#include "Terrain.h"
#include "Window.h"
#include "Camera.h"
#include "Common.h"

class Scene
{
public:
	//Constructor and destructor
	Scene() : terrain()  {};
	~Scene() {};
	//Pointer to active camera
	Camera* activeCamera;
	//Terrain
	Terrain terrain;
	//Draw scene to GLFW window
	void Draw(const Window&);
	void DrawTerrainNode(
		const Window& window, const QuadTree<TerrainNode>::Iterator& node
		) const;
	//Wireframe settings
	float wireframeThickness = 0.001f;
	vec3 wireframeColor = vec3(0.0f);
};

#endif //SCENE_H

