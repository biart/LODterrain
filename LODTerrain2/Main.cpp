/*
	This file defines the entry point of application
*/

#include "Shader.h"
#include "Scene.h"
#include "Camera.h"
#include "Window.h"
#include <vector>

void ProcessCamera(Camera& cam, const Window& window, float& speed)
{
	speed *= pow(1.5f, window.GetScrollingSpeed());

	vec3 shift = vec3();
	if (window.IsKeyPressed(GLFW_KEY_W))
		shift += vec3(0.0f, 0.0f, 1.0f);
	if (window.IsKeyPressed(GLFW_KEY_S))
		shift -= vec3(0.0f, 0.0f, 1.0f);
	if (window.IsKeyPressed(GLFW_KEY_D))
		shift += vec3(1.0f, 0.0f, 0.0f);
	if (window.IsKeyPressed(GLFW_KEY_A))
		shift -= vec3(1.0f, 0.0f, 0.0f);
	if (window.IsKeyPressed(GLFW_KEY_UP))
		shift += vec3(0.0f, 1.0f, 0.0f);
	if (window.IsKeyPressed(GLFW_KEY_DOWN))
		shift -= vec3(0.0f, 1.0f, 0.0f);
	if (length(shift) > FLT_EPSILON)
		shift = speed*normalize(shift);
	cam.Move(shift);

	cam.orientation.x += window.GetMouseSpeed().y;
	cam.orientation.y += window.GetMouseSpeed().x;
}

int main(void)
{
	Window window;
	ChangeLog("LODTerrain.log");
	if(!window.Create(uvec2(800, 600), "OpenGL")) 
		return EXIT_FAILURE;
	OpenGLPrintDebugInfo();
	WriteToLog("\n");

	/*
		Load shaders
	*/
    if(!window.CreateShaderProgram("default.vsh", "default.fsh", "default.gsh"))
        return 1;
	window.program.Use();
	WriteToLog("OK: Program is used now\n");
	OPENGL_CHECK_FOR_ERRORS();
	WriteToLog("OK: Renderer is ready\n");

	/*
		Main loop
	*/
	Scene scene;
	Camera cam;
	float camSpeed = 0.1f;

	scene.activeCamera = &cam;
	cam.FOV = 45.0f;
	cam.position = glm::vec3(0.0f, 20.0f, 0.0f);
	scene.terrain.LoadFromFile("land.tga");
	scene.terrain.position = vec3(20.0f, 0.0f, 10.0f);
	scene.terrain.scale = vec3(60.0f, 25.0f, 60.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	glClearDepth(1.0f);
	OPENGL_CHECK_FOR_ERRORS();

	WriteToLog("Working...\n");
	unsigned int counter = 0;

	unsigned int previousTime = GetTime();
	unsigned int previousCount = 0;
	unsigned int fps = 0;
	while (!window.ShouldClose())
	{
		unsigned int currentTime = GetTime();
		if (previousTime < GetTime())
		{
			fps = counter - previousCount;
			previousTime = GetTime();
			previousCount = counter;
		}
		window.SetTitle(
			string("FPS: ") +
			ToString(fps) +
			string(" | Current position: (") +
			ToString(scene.activeCamera->position.x) + string(", ") +
			ToString(scene.activeCamera->position.y) + string(", ") +
			ToString(scene.activeCamera->position.z) + string(")")
			);

		ivec2 size = window.GetSize();
		glViewport(0, 0, size.x, size.y);
		cam.aspect = static_cast<float>(size.x) / static_cast<float>(size.y);

		if (counter % 20 == 0)
			scene.terrain.Renew(scene.activeCamera->position);
		scene.Draw(window);

		window.PollEvents();
		ProcessCamera(cam, window, camSpeed);
		if (window.GetStrokedKey() == GLFW_KEY_SPACE)
			scene.terrain.showGrid = !scene.terrain.showGrid;
		if (window.GetStrokedKey() == GLFW_KEY_Q)
			window.FixCursor(!window.IsCursorFixed());

		counter++;
	}
	OPENGL_CHECK_FOR_ERRORS();

	/*
		Terminate program
	*/
	scene.terrain.Unload();
	window.Destroy();
	WriteToLog("DONE.\n");
	return 0;
}