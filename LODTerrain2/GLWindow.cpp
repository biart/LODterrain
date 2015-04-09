#include "GLWindow.h"

GLWindow::GLWindow(void)
{
	for(int i = 0; i < 256; i++)
		keys[i] = false;
	mouseSensivity = 0.1f;
	wheelDelta = 0.0f;
	cameraSpeed = 0.05f;
	running = active = false;
}

GLWindow::~GLWindow(void)
{
	Destroy();
}

//
//Cursor movement processing
// Refreshes vector of pointer speed (GLWindow::cursorSpeed)
// and attaches it to the center of screen 
//
void GLWindow::ProcessCursor(void)
{
	if (fixedCursor)
	{
		const int centerx = GetSystemMetrics(SM_CXSCREEN) / 2;
		const int centery = GetSystemMetrics(SM_CYSCREEN) / 2;
		POINT curpos;
		GetCursorPos(&curpos);
		SetCursorPos(centerx, centery);
		cursorSpeed = glm::vec2((float)(curpos.x - centerx), -(float)(curpos.y - centery)) * mouseSensivity;
	}
}

bool GLWindow::Create(const char *clname, const char *title, int w, int h, bool fullscr = false)
{
	//Firtly try to destroy previous window
	Destroy();
	WriteToLog("Creating window with OpenGL support...\n");

	//Validate the arguments
	w = abs(w);
	h = abs(h);

	if(!clname)
	{
		WriteToLog("ERROR: empty window class name (%d)\n", GetLastError());
		return false;
	}

	if(!title)
	{
		WriteToLog("ERROR: empty title (%d)\n", GetLastError());
		return false;
	}

	WNDCLASSEX            wcx;
	PIXELFORMATDESCRIPTOR pfd;
	RECT                  rect;
	HGLRC                 hRCTemp;
	DWORD                 style, exStyle;
	int                   x, y, format;

	size = uvec2();
	strcpy_s(classname, 256, clname);

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;

	//Set extended context attributes
	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	//Get instance handle
	hInstance = static_cast<HINSTANCE>(GetModuleHandle(NULL));

	//Register window class
	memset(&wcx, 0, sizeof(wcx));
	wcx.cbSize        = sizeof(wcx);
	wcx.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcx.lpfnWndProc   = (WNDPROC)StaticWndProc;
	wcx.hInstance     = hInstance;
	wcx.lpszClassName = classname;
	wcx.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wcx.hCursor       = LoadCursor(NULL, IDC_ARROW);

	if (!RegisterClassEx(&wcx))
	{
		WriteToLog("ERROR: RegisterClassEx fail (%d)\n", GetLastError());
		return false;
	}
	else
		WriteToLog("OK: Window class was registered\n");

	//Set window style
	style   = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	exStyle = WS_EX_APPWINDOW;

	//Center the window
	x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
	y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;

	rect.left   = x;
	rect.right  = x + w;
	rect.top    = y;
	rect.bottom = y + h;

	//Adjust window size to style
	AdjustWindowRectEx (&rect, style, FALSE, exStyle);

	//Directly create the window
	hWnd = CreateWindowEx(exStyle, classname, title, style, rect.left, rect.top, 
		rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, this);
	if (!hWnd)
	{
		WriteToLog("ERROR: CreateWindowEx fail (%d)\n", GetLastError());
		return false;
	}
	else
		WriteToLog("OK: Window was successfully created\n");

	//Get handle of device context
	hDC = GetDC(hWnd);
	if (!hDC)
	{
		WriteToLog("ERROR: GetDC fail (%d)\n", GetLastError());
		return false;
	}

	//Set basic pixel attributes
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize      = sizeof(pfd);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;

	//Choose proper pixel format
	format = ChoosePixelFormat(hDC, &pfd);
	if (!format || !SetPixelFormat(hDC, format, &pfd))
	{
		WriteToLog("ERROR: Setting pixel format fail (%d)\n", GetLastError());
		return false;
	}
	else
		WriteToLog("OK: Pixel format was set successfully\n");

	//Create temporary render context
	hRCTemp = wglCreateContext(hDC);
	if (!hRCTemp || !wglMakeCurrent(hDC, hRCTemp))
	{
		WriteToLog("ERROR: Ñreating temp render context fail (%d)\n", GetLastError());
		return false;
	}
	else
		WriteToLog("OK: Temporary render context was created\n");

	wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(
		wglGetProcAddress("wglCreateContextAttribsARB")
		);

	//Destroy temporary render context
	// (we need to create atavistic temporary RC to obtain "wglCreateContextAttribsARB" function)
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRCTemp);
	if (!wglCreateContextAttribsARB)
	{
		WriteToLog("ERROR: wglCreateContextAttribsARB fail (%d)\n", GetLastError());
		return false;
	}
	else
		WriteToLog("OK: wglCreateContextAttribsARB was delivered from driver\n");
	
	//Create extended render context
	hRC = wglCreateContextAttribsARB(hDC, 0, attribs);
	if (!hRC || !wglMakeCurrent(hDC, hRC))
	{
		WriteToLog("ERROR: Creating render context fail (%d)\n", GetLastError());
		return false;
	}
	else
		WriteToLog("OK: New render context was created\n");

	WriteToLog("\n");
	OpenGLPrintDebugInfo();
	WriteToLog("\n");

	//Set window size
	SetSize(uvec2(w, h), fullscr);

	//Initialize OpenGL extentions (use glew to make it automatically)
	OpenGLInitExtensions();

	WriteToLog("OK: OpenGL window was succesfully created\n");

	return true;
}

void GLWindow::Destroy()
{
	WriteToLog("Destroying window...\n");
	//Restore the screen resolution
	if (fullScreen)
	{
		ChangeDisplaySettings(NULL, CDS_RESET);
		WriteToLog("Fullscreen disabled\n");
	}
	ShowCursor(TRUE);

	//Delete render context
	if (hRC)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hRC);
		WriteToLog("OK: Context was destroyed\n");
	}

	//Release device context
	if (hDC)
	{
		ReleaseDC(hWnd, hDC);
		WriteToLog("OK: DC was released\n");
	}

	//Delete the window
	if (hWnd)
	{
		DestroyWindow(hWnd);
		WriteToLog("OK: Window was destroyed\n");
	}

	//Delete window class
	if (hInstance)
		UnregisterClass(classname, hInstance);

	for (int i = 0; i < 256; i++)
		keys[i] = false;
	mouseSensivity = 0.1f;
	wheelDelta = 0.0f;

	WriteToLog("OK: Window destroyed\n");
}

void GLWindow::SetSize(uvec2 sz, bool fullscr)
{
	RECT    rect;
	DWORD   style, exStyle;
	DEVMODE devMode;
	LONG    result;
	int     x, y;

	//Restore the screen resolution
	if (fullScreen && !fullscr)
	{
		ChangeDisplaySettings(NULL, CDS_RESET);
		WriteToLog("Fullscreen disabled\n");
	}

	fullScreen = fullscr;

	//Set the fullscreen mode
	if (fullScreen)
	{
		memset(&devMode, 0, sizeof(devMode));
		devMode.dmSize       = sizeof(devMode);
		devMode.dmPelsWidth  = sz.x;
		devMode.dmPelsHeight = sz.y;
		devMode.dmBitsPerPel = GetDeviceCaps(hDC, BITSPIXEL);
		devMode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;

		result = ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
		if (result != DISP_CHANGE_SUCCESSFUL)
		{
			WriteToLog("ERROR: ChangeDisplaySettings failed %dx%d (%d)\n", sz.x, sz.y, result);
			fullScreen = false;
		}
		else
			WriteToLog("OK: Display settings was changed. Fullscreen mode enabled\n");
	}

	if (fullScreen)
	{
		style   = WS_POPUP;
		exStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;

		x = y = 0;
	} else
	{
		style   = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		exStyle = WS_EX_APPWINDOW;

		x = (GetSystemMetrics(SM_CXSCREEN) - sz.x) / 2;
		y = (GetSystemMetrics(SM_CYSCREEN) - sz.y) / 2;
	}

	rect.left   = x;
	rect.right  = x + sz.x;
	rect.top    = y;
	rect.bottom = y + sz.y;

	AdjustWindowRectEx (&rect, style, FALSE, exStyle);

	SetWindowLong(hWnd, GWL_STYLE,   style);
	SetWindowLong(hWnd, GWL_EXSTYLE, exStyle);

	SetWindowPos(hWnd, HWND_TOP, rect.left, rect.top,
		rect.right - rect.left, rect.bottom - rect.top,
		SWP_FRAMECHANGED);

	//Show window
	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	UpdateWindow(hWnd);

	WriteToLog("Window was resized\n");

	//Get window size
	GetClientRect(hWnd, &rect);
	size = uvec2(rect.right - rect.left, rect.bottom - rect.top);

	WriteToLog("Width = %d, Height = %d\n", size.x, size.y);

	//Set the viewport
	OPENGL_CALL(glViewport(0, 0, size.x, size.y));
	OPENGL_CALL(glEnable(GL_CULL_FACE));

	//Center the cursor
	SetCursorPos(x + size.x / 2, y + size.y / 2);

	OPENGL_CHECK_FOR_ERRORS();
}

void GLWindow::ProcessMessages()
{
	stroked_key = 0;
	MSG msg;
	while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			running = false;
			break;
		}
		//TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void GLWindow::ProcessCamera(Camera& cam)
{
	//Move camera by pressing keys
	vec3 shift = vec3();
	if (keys[VK_UP])
		shift += vec3(0.0f, 0.0f, 1.0f);
	if (keys[VK_DOWN])
		shift -= vec3(0.0f, 0.0f, 1.0f);
	if (keys[VK_RIGHT])
		shift += vec3(1.0f, 0.0f, 0.0f);
	if (keys[VK_LEFT])
		shift -= vec3(1.0f, 0.0f, 0.0f);
	if (keys[VK_ADD])
		shift += vec3(0.0f, 1.0f, 0.0f);
	if (keys[VK_SUBTRACT])
		shift -= vec3(0.0f, 1.0f, 0.0f);
	if (length(shift) > FLT_EPSILON)
		shift = cameraSpeed*normalize(shift);
	cam.Move(shift);
	//Move camera by scrolling mouse wheel
	cameraSpeed *= pow(2.0f, wheelDelta/2000.0f);
	if (cameraSpeed < 0)
		cameraSpeed = 0;
	//Rotate camera by moving mouse
	cam.orientation.y += cursorSpeed.x;
	cam.orientation.x -= cursorSpeed.y;
}

bool GLWindow::LoadRenderer()
{
	WriteToLog("Loading shaders and preparing renderer...\n");
	// Setting OpenGL parameters
	OPENGL_CALL(glEnable(GL_DEPTH_TEST));
    glClearColor (1.0, 1.0, 1.0, 0.0);
	glClearDepth(1.0f);

	// Setting shaders
	pProgram = new GLProgram();
	pVertShader = new GLShader();
	pFragShader = new GLShader();
 
	if(!pVertShader->CreateFromFile("default.vsh", GL_VERTEX_SHADER))
		return false;

	WriteToLog("OK: Vertex shader creation is complete\n");

	if(!pFragShader->CreateFromFile("default.fsh", GL_FRAGMENT_SHADER))
		return false;

	WriteToLog("OK: Fragment shader creation is complete\n");
 
	pProgram->AttachShader(pVertShader);
	pProgram->AttachShader(pFragShader);

	WriteToLog("OK: Shaders attached to program\n");

	glBindAttribLocation(pProgram->program, 0, "in_Position");
	glBindAttribLocation(pProgram->program, 1, "in_Color");

	if(!pProgram->Link())
		return false;

	WriteToLog("OK: Program was linked\n");

	if(!pProgram->Validate())
		return false;

	WriteToLog("OK: Program creation is complete\n");

	pProgram->Use();	

	OPENGL_CHECK_FOR_ERRORS();

	WriteToLog("OK: Program is used now\n");

	WriteToLog("OK: Renderer is ready\n");

	return true;
}

void GLWindow::UnloadRenderer()
{
	WriteToLog("Destroying shaders and unloading renderer...\n");

	delete pProgram;
	pProgram = NULL;
 
	delete pVertShader;
	pVertShader = NULL;
	delete pFragShader;
	pFragShader = NULL;
 
	wglMakeCurrent(NULL, NULL);
	if(hRC)
	{
		wglDeleteContext(hRC);
		hRC = NULL;
	}

	WriteToLog("OK: Renderer is unloaded\n");
}

void GLWindow::DrawScene(Scene* s)
{
	const mat4 worldmatrix(
		1.0f, 0.0f, 0.0f, 0.0f, // x-axis is pointing to the right
		0.0f, 1.0f, 0.0f, 0.0f, // y-axis is pointing up
		0.0f, 0.0f,-1.0f, 0.0f, // and z-axis is pointing to the front of us
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// Enable depth test
	OPENGL_CALL(glDepthFunc(GL_LESS));
	// Clear buffer
	OPENGL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// Get view projection matrix
	mat4 vpmatrix = s->activeCamera->GetProjectionMatrix() * worldmatrix * s->activeCamera->GetViewMatrix();
	mat4 mvpmatrix;

	//
	// Render terrain
	//
	mvpmatrix = vpmatrix * s->terrain.GetModelMatrix();
	//load matrix to GPU memory as uniform
	glUniformMatrix4fv(
		glGetUniformLocation(pProgram->program, "modelViewProjectionMatrix"), 
		1,			//we have only one matrix calculated by CPU. You can easily shift these calculations to GPU
		GL_FALSE,	//we want to use traditional matrix arithmetic so we needn't to transpose matrices
		value_ptr(mvpmatrix)); //Our matrix

	//draw elements of the terrain
	//currently loaded shader program will process all of these elements
	if (s->terrain.showSurface)
	{
		glBindVertexArray(s->terrain.GetVAO());
		glBindBuffer(GL_ARRAY_BUFFER, s->terrain.GetColorsVBO());
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(1);
		glDrawElements(GL_TRIANGLES, s->terrain.GetIndicesCount(), GL_UNSIGNED_INT, NULL); // draw colored surface
	}
	if (s->terrain.showGrid)
	{
		glDepthFunc(GL_ALWAYS); // disable depth test
		glDisableVertexAttribArray(1); // disable vertex attribute array
		                               // now we can use glVertexAttrib*** functions 
		                               // to provide the default vertex attribute 
		glVertexAttrib3f(1, 0.0f, 0.0f, 0.0f); // set default color of all vertices to (0, 0, 0)
		glDrawElements(GL_LINES, s->terrain.GetIndicesCount(), GL_UNSIGNED_INT, NULL); // draw grid
	}
	// swap buffers and show result on the screen
	SwapBuffers(hDC);

	OPENGL_CHECK_FOR_ERRORS();
}

//
//Static WndProc, that takes this-pointer as lparam and calls non-static method this->WndProc(..)
//
LRESULT CALLBACK GLWindow::StaticWndProc(HWND hWndl, UINT msg, WPARAM wParam, LPARAM lParam)
{
	GLWindow* pWindow;
	if( msg == WM_NCCREATE )
	{
		pWindow = (GLWindow*)(((CREATESTRUCT*)lParam)->lpCreateParams);
		::SetWindowLong( hWndl, GWL_USERDATA, reinterpret_cast<LONG>(pWindow) );
	}
	else
		pWindow = reinterpret_cast<GLWindow*>(::GetWindowLong( hWndl, GWL_USERDATA));

	if( !pWindow ) return
		::DefWindowProc( hWndl, msg, wParam, lParam );
	return pWindow->WndProc(hWndl, msg, wParam, lParam);
}

//
//WndProc "method" 
//
LRESULT CALLBACK GLWindow::WndProc(HWND hWndl, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//Process caught message
	switch (msg)
	{
		//Key pressed:
		case WM_KEYDOWN:
			//WriteToLog("WM_KEYDOWN message is processing\n");

			if (wParam == VK_ESCAPE)
				running = false;
			else
			{
				if(!(lParam & (1 << 29)))
					stroked_key = wParam;
				else
					stroked_key = 0;
				keys[wParam] = true;
			}
			return FALSE;
		case WM_KEYUP:
			//WriteToLog("WM_KEYUP message is processing\n");

			keys[wParam] = false;
			return FALSE;
		//Mouse wheel scrolled:
		case WM_MOUSEWHEEL:
			//WriteToLog("WM_MOUSEWHEEL message is processing\n");

			wheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
			return FALSE;
		//Window focus changed:
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
			//WriteToLog("WM_SETFOCUS or WM_KILLFOCUS message is processing\n");
			active = (msg == WM_SETFOCUS);
			return FALSE;
		//Window activated/deactivated:
		case WM_ACTIVATE:
			//WriteToLog("WM_ACTIVATE message is processing\n");
			active = (LOWORD(wParam) != WA_INACTIVE);
			return FALSE;
		//Window closed (Alt+F4, [X], e.t.c):
		case WM_CLOSE:
			//WriteToLog("WM_CLOSE message is processing\n");
			running = active = false;
			PostQuitMessage(0);
			return FALSE;
		//System command caught:
		case WM_SYSCOMMAND:
			//WriteToLog("WM_SYSCOMMAND message is processing\n");
			switch (wParam & 0xFFF0)
			{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					if (fullScreen)
						return FALSE;
					break;

				case SC_KEYMENU:
					return FALSE;
			}
			break;
		//Window background should be erased:
		case WM_ERASEBKGND:
			//WriteToLog("WM_ERASEBKGND message is processing\n");
			return FALSE;
	}
	//Default message processor (should be called anyway)
	return ::DefWindowProc(hWndl, msg, wParam, lParam);
}