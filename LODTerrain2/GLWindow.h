/*
	GLWindow class
	Window with an OpenGL support
*/

#ifndef GLWINDOW_H
#define GLWINDOW_H

#include "Common.h"
#include "Scene.h"
#include "GLShader.h"
#include "Camera.h"
#include <list>

class GLWindow
{
public:
	//Ñonstructor and destructor
	GLWindow(void);
	~GLWindow(void);

	//Initialize and show window
	bool Create(const char *clname, const char *title, int width, int height, bool fullScreen);

	//Hide and destroy window
	void Destroy();

	//Set window size
	void SetSize(uvec2 size, bool fullScreen = false);
	//Get window size
	uvec2 GetSize() const { return size; };

	//Cursor processing functions
	//(returns cursor to the center of screen, but saves the offset of its coordinates) 
	void ProcessCursor(void);
	//Camera processing functions
	//(moves camera according to the movement of cursor)
	void ProcessCamera(Camera& cam);
	//Processing windows messages
	//(such as quit messages or messages from keyboard)
	void ProcessMessages();
	LRESULT CALLBACK WndProc(HWND hWndl, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK StaticWndProc(HWND hWndl, UINT msg, WPARAM wParam, LPARAM lParam);

	//Shaders preparation stuff
	bool LoadRenderer();
	//Shaders destroying
	void UnloadRenderer();

	//Key processing arrays
	bool keys[256]; // pressed keys
	unsigned char stroked_key; // recently stroked key

	//Mouse wheel offset
	// (should be probably set to null manually at the end of each loop)
	float wheelDelta;

	//Draw scene
	void DrawScene(Scene* s);
	
	//Get or set state
	void Run() { active = running = true; } // window is disabled until this method is called
	bool IsActive() const { return active; } // is active
	bool IsRunning() const { return running; } // is still running
	operator bool() const { return running; }  //
	bool IsFullScreen() const { return fullScreen; } // is in full screen mode
	void FixCursor() 
	{  
		cursorSpeed = vec2();
		fixedCursor = !fixedCursor;
		if (fixedCursor)
			ShowCursor(FALSE);
		else
			ShowCursor(TRUE);
	};

private:
	//Copy constructor and assignment operator
	// (copying should be inaccessible)
	GLWindow(const GLWindow& obj) {}
	GLWindow& operator=(const GLWindow& obj) {}

	//Shaders and shader programs
	GLProgram* pProgram;
	GLShader* pVertShader; // vertex shader
	GLShader* pFragShader; // program shader

	//Window size
	uvec2 size;

	//Mouse sensivity
	float mouseSensivity;

	//Camera movement speed;
	float cameraSpeed;

	//Current speed of the cursor
	glm::vec2 cursorSpeed;

	//Flags
	bool fullScreen;
	bool active;
	bool running;
	bool fixedCursor;

	//Other identifiers
	HINSTANCE hInstance; // instance handler
	HWND      hWnd;      // window handler
	HDC       hDC;       // device context handler
	HGLRC     hRC;       // render context handler
	char	  classname[256]; // window class name
};

#endif //GLWINDOW_H