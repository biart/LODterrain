#ifndef WINDOW_H
#define WINDOW_H

#include "Common.h"
#include "Shader.h"
#include <vector>

const int numberOfKeys = GLFW_KEY_LAST + 1;

class Window
{
public:

	//Constructor and destructor
	Window();
	~Window();

	//Create window and OpenGL context
	bool Create(uvec2 size, const string& title, bool fullscreen = false);
	//Destroy window
	void Destroy();

	//Returns if specified key is pressed
	bool IsKeyPressed(int keyid) const { return glfwGetKey(glwindow, keyid) == GLFW_PRESS; }
	//Returns recently stroked key or GLFW_KEY_UNKNOWN if no keys were clicked
	int GetStrokedKey() const { return strokedKey; }

	//Fix or unfix the cursor. Fixed cursor is bounded to the center of screen and hid. 
	//Mouse speed is only calculating when cursor is fixed
	void FixCursor(bool fixed);
	//Returns if the cursor is hidden
	bool IsCursorFixed() const { return fixedCursor; }
	//Returns cursor speed
	vec2 GetMouseSpeed() const { return mouseSpeed; }
	//Returns wheel offset
	float GetScrollingSpeed() const { return scrollingSpeed; }
	//Swap buffers
	void SwapBuffers() const { glfwSwapBuffers(glwindow); }
	//If the window should close
	bool ShouldClose() const { return glfwWindowShouldClose(glwindow); }
	//Processing messages
	void PollEvents() const;
	//Get framebuffer size
	ivec2 GetSize() const 
	{ 
		ivec2 size;
		glfwGetFramebufferSize(glwindow, &size.x, &size.y);
		return size;
	};
	//Set title
	void SetTitle(const string& title)
	{
		glfwSetWindowTitle(glwindow, title.c_str());
	}

    //Create shader program from sources
    bool CreateShaderProgram(const std::string& vertFilename, const std::string& fragFilename, const std::string& geomFilename);

	//Rendering stuff
	Program program;
	Shader vertShader;
	Shader fragShader;
	Shader geomShader;

	//Input parameters
	static float mouseSensivity;

private:

	//Input handlers
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void error_callback(int error, const char* description);
	static void cursor_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	//Static input variables
	static int strokedKey;
	static vec2 mouseSpeed;
	static float scrollingSpeed;
	static bool fixedCursor;
	static bool notReleased;
	GLFWwindow* glwindow;
};

#endif // WINDOW_H
