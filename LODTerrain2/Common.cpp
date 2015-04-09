#include "Common.h"

GLenum g_OpenGLError = GL_NO_ERROR;

const int LOGGER_FILENAME_MAX              = 256;
char g_LoggerFileName[LOGGER_FILENAME_MAX] = "log.txt";

unsigned int GetTime()
{
	return time(nullptr);
}

signed char dcompare(double a, double b, double eps)
{
	if(a > b + eps)
		return 1;
	if(a < b - eps)
		return -1;
	return 0;
}

signed char fcompare(float a, float b, float eps)
{
	if(a > b + eps)
		return 1;
	if(a < b - eps)
		return -1;
	return 0;
}

void ChangeLog(const char *fileName)
{
	FILE *output;

	memset(g_LoggerFileName, 0, LOGGER_FILENAME_MAX);
	strncpy(g_LoggerFileName, fileName, LOGGER_FILENAME_MAX - 1);

	if ((output = fopen(g_LoggerFileName, "w")) != NULL)
		fclose(output);
}

void WriteToLog(const char *format, ...)
{
	va_list ap;
	FILE    *output;

	if ((output = fopen(g_LoggerFileName, "a+")) == NULL)
		return;

	va_start(ap, format);
	vfprintf(output, format, ap);
	va_end(ap);

	fclose(output);
}

bool LoadFile(const char *fileName, bool binary, uint8_t **buffer, uint32_t *size)
{
	if(!fileName)
	{
		return false;
	}
	if(!buffer)
	{
		return false;
	}
	if(!size)
	{
		return false;
	}

	FILE     *input;
	uint32_t fileSize, readed;

	const char mode[] = {'r', binary ? 'b' : 't', '\0'};

	if ((input = fopen(fileName, mode)) == NULL)
	{
		return false;
	}

	fseek(input, 0, SEEK_END);
	fileSize = (uint32_t)ftell(input);
	rewind(input);

	if (fileSize == 0)
	{
		WriteToLog("ERROR: Empty file '%s'\n", fileName);
		fclose(input);
		return false;
	}

	*buffer = new uint8_t[fileSize];
	if(!*buffer)
	{
		return false;
	}

	readed = fread(*buffer, 1, fileSize, input);

	fclose(input);

	if (readed != fileSize)
	{
		WriteToLog("ERROR: Can't read file '%s'\n", fileName);
		delete[] *buffer;
		return false;
	}

	*size = fileSize;

	return true;
}

void OpenGLPrintDebugInfo()
{
	// OpenGL context info
	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	WriteToLog("OpenGL render context information:\n"
		"  Renderer       : %s\n"
		"  Vendor         : %s\n"
		"  Version        : %s\n"
		"  GLSL version   : %s\n"
		"  OpenGL version : %d.%d\n",
		(const char*)glGetString(GL_RENDERER),
		(const char*)glGetString(GL_VENDOR),
		(const char*)glGetString(GL_VERSION),
		(const char*)glGetString(GL_SHADING_LANGUAGE_VERSION),
		major, minor
	);

	// Important OpenGL parameters
	OPENGL_INT_PRINT_DEBUG(GL_MAX_VERTEX_ATTRIBS);
	OPENGL_INT_PRINT_DEBUG(GL_MAX_TEXTURE_IMAGE_UNITS);

	OPENGL_CHECK_FOR_ERRORS();
}

