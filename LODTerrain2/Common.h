//
//This header contains the most common definitions used in the project
//

#ifndef COMMON_H
#define COMMON_H

#define _CRT_SECURE_NO_WARNINGS

// OpenGL headers
#define GLEW_STATIC 
#include "GL/glew.h"
#include "GLFW/glfw3.h"

// Standart C headers
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <cmath>
#include <ctime>

#include <sstream>

#if defined(_MSC_VER)
#define snprintf _snprintf
#elif defined(__MINGW32__)
#include <stdint.h>
#else
#error You must compile this code with MSVC or MinGW!
#endif

// OpenGL math extensions
// Contains analogues of shader computation stuff, such as
//  - linear algebra functions
//  - vector types and vector arithmetic
//  - matrices and their transformations
#include "glm\glm.hpp"
#include "glm\ext.hpp"

// Using STL and GLM namespaces
using namespace std;
using namespace glm;

// Floating point values comparation with a given precision
signed char fcompare(float a, float b, float eps = FLT_EPSILON);
signed char dcompare(double a, double b, double eps = DBL_EPSILON);

// Safety load file to buffer
bool LoadFile(const char *fileName, bool binary, uint8_t **buffer, uint32_t *size);

// Change log path
void ChangeLog(const char *fileName);

// Write to current log file
void WriteToLog(const char *format, ...);

// Global variable for OpenGL error storage
extern GLenum g_OpenGLError;

// Get current time
unsigned int GetTime();

//Convert integer to string 
template <typename T>
string ToString(T val)
{
	ostringstream sout;
	sout << val;
	return string(sout.str());
}



// Print debug info
#define OPENGL_INT_PRINT_DEBUG(name) \
	GLint info_ ## name; \
	glGetIntegerv(name, &info_ ## name); \
	WriteToLog(#name " = %d\n", info_ ## name);

// Safety call of GetProc
#define OPENGL_GET_PROC(p,n) \
	n = (p)wglGetProcAddress(#n); \
	if (NULL == n) \
	{ \
		WriteToLog("ERROR: Loading extension '%s' fail (%d)\n", #n, GetLastError()); \
		return false; \
	}

// Check for OpenGL errors
#define OPENGL_CHECK_FOR_ERRORS() \
	if ((g_OpenGLError = glGetError()) != GL_NO_ERROR) \
		WriteToLog("ERROR: OpenGL error %d\n", (int)g_OpenGLError);

// Safety call of OpenGL function
#define OPENGL_CALL(expression) \
	{ \
		expression; \
		if ((g_OpenGLError = glGetError()) != GL_NO_ERROR) \
			WriteToLog("ERROR: OpenGL expression \"" #expression "\" error %d\n", (int)g_OpenGLError); \
	}

// Initialization of neccesary OpenGL extentios
bool OpenGLInitExtensions();

// Print to log the info about OpenGL version and state
void OpenGLPrintDebugInfo();

#endif // COMMON_H