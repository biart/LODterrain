#include "Window.h"

bool Window::fixedCursor = false;
vec2 Window::mouseSpeed = vec2(0.0f, 0.0f);
int Window::strokedKey = GLFW_KEY_UNKNOWN;
bool Window::notReleased = false;
float Window::scrollingSpeed = 0.0f;
float Window::mouseSensivity = 0.2f;

void Window::error_callback(int error, const char* description)
{
	WriteToLog("ERROR: %s (%d)", description, error);
}

void Window::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	strokedKey = GLFW_KEY_UNKNOWN;
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key != GLFW_KEY_UNKNOWN)
		{
			strokedKey = key;
		}
		else
			WriteToLog("ERROR: Unknown key pressed.");
	}
}

void Window::cursor_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (fixedCursor)
	{
		mouseSpeed = vec2(xpos, ypos) * mouseSensivity;
		glfwSetCursorPos(window, 0.0, 0.0);
	}
	else
	{
		mouseSpeed = vec2(0.0f, 0.0f);
	}
}

void Window::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	scrollingSpeed = yoffset;
}

Window::Window()
{
}

Window::~Window()
{
}

bool Window::Create(uvec2 size, const string& title, bool fullscreen)
{
	/*
		Create a window with OpenGL context
	*/
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		glfwTerminate();
		WriteToLog("ERROR: GLFW initialization failed\n");
		return false;
	}
	glfwWindowHint(GLFW_SAMPLES, 16);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (fullscreen)
		glwindow = glfwCreateWindow(size.x, size.y, title.c_str(), glfwGetPrimaryMonitor(), nullptr);
	else
		glwindow = glfwCreateWindow(size.x, size.y, title.c_str(), nullptr, nullptr);
	if (!glwindow)
	{
		glfwTerminate();
		WriteToLog("ERROR: Window creation failed\n");
		return false;
	}
	glfwMakeContextCurrent(glwindow);
	glfwSetKeyCallback(glwindow, key_callback);
	glfwSetCursorPosCallback(glwindow, cursor_callback);
	glfwSetScrollCallback(glwindow, scroll_callback);

	/*
		Initialize extensions
	*/
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		WriteToLog("ERROR: GLEW initialization failed\n");
		glfwTerminate();
		WriteToLog("ERROR: %s\n", glewGetErrorString(err));
		return false;
	}
	WriteToLog("OK: Window was successfully initialized\n");
	return true;
}

void Window::Destroy()
{
	glfwDestroyWindow(glwindow);
	strokedKey = GLFW_KEY_UNKNOWN;
	WriteToLog("OK: Window was destroyed\n");
}

void Window::FixCursor(bool fixed)
{
	if (fixed)
	{
		ivec2 size = GetSize();
		glfwSetCursorPos(glwindow, 0.0, 0.0);
		glfwSetInputMode(glwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		fixedCursor = true;
	}
	else
	{
		glfwSetInputMode(glwindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		fixedCursor = false;
	}
}

void Window::PollEvents() const 
{ 
	mouseSpeed = vec2(0.0f, 0.0f); 
	strokedKey = GLFW_KEY_UNKNOWN; 
	scrollingSpeed = 0.0f;
	glfwPollEvents(); 
}

bool Window::CreateShaderProgram(const std::string& vertFilename, const std::string& fragFilename, const std::string& geomFilename = "")
{
    if (!vertShader.CreateFromFile(vertFilename, GL_VERTEX_SHADER))
        return false;
    WriteToLog("OK: Vertex shader creation is complete\n");

    if (geomFilename.length() > 0)
    {
        if (!geomShader.CreateFromFile(geomFilename, GL_GEOMETRY_SHADER))
            return false;
        WriteToLog("OK: Geometry shader creation is complete\n");
    }

    if (!fragShader.CreateFromFile(fragFilename, GL_FRAGMENT_SHADER))
        return false;
    WriteToLog("OK: Fragment shader creation is complete\n");

    program.AttachShader(vertShader);
    program.AttachShader(geomShader);
    program.AttachShader(fragShader);
    WriteToLog("OK: Shaders attached to program\n");

    if (!program.Link())
        return false;
    WriteToLog("OK: Program was linked\n");
    if (!program.Validate())
        return false;
    WriteToLog("OK: Program creation is complete\n");

    return true;
}