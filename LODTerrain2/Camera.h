/*
	Camera class
*/

#ifndef CAMERA_H
#define CAMERA_H

#include "Common.h"

class Camera
{
public:
	//Constructor and destructor
	Camera(void) {};
	~Camera(void) {};

	//Move along a direction
	void Move(vec3 shift);

	//Get matrices corresponding to the camera
	mat4 GetViewMatrix(void) const;
	mat4 GetProjectionMatrix(void) const;

	//Position and orientation in 3D-space
	vec3 position = vec3(0.0f);
	vec3 orientation = vec3(0.0f);

	//View parameters
	float FOV = 60.0f;
	float aspect = 1.0f;
};

#endif // CAMERA_H

