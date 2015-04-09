#include "Camera.h"

mat4 Camera::GetViewMatrix(void) const
{
	mat4 mmatrix = mat4(1.0f);
	mmatrix *= rotate(mat4(1.0f), -orientation.x, vec3(1.0f, 0.0f, 0.0f));
	mmatrix *= rotate(mat4(1.0f), -orientation.y, vec3(0.0f, 1.0f, 0.0f));
	mmatrix *= rotate(mat4(1.0f), -orientation.z, vec3(0.0f, 0.0f, 1.0f));
	return translate(mmatrix, -position);
}

mat4 Camera::GetProjectionMatrix(void) const
{
	return perspective(FOV, aspect, 0.1f, 10000.0f);
}

void Camera::Move(vec3 shift)
{
	mat4 mmatrix = mat4(1.0f);
	mmatrix *= rotate(mat4(1.0f), orientation.z, vec3(0.0f, 0.0f, 1.0f));
	mmatrix *= rotate(mat4(1.0f), orientation.y, vec3(0.0f, 1.0f, 0.0f));
	mmatrix *= rotate(mat4(1.0f), orientation.x, vec3(1.0f, 0.0f, 0.0f));
	position += vec3(mmatrix * vec4(shift, 1.0f));
}