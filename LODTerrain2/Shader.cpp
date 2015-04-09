#include "Shader.h"

//
//Shader class constructor
//
Shader::Shader(void)
{
	shader = 0;
}

//
//Shader class destructor
//
Shader::~Shader(void)
{
	if(shader)
		glDeleteShader(shader);
}

//
//Program class constructor
//
Program::Program(void)
{

}

//
//Program class destructor
//
Program::~Program(void)
{
	if(program)
		glDeleteProgram(program);
}

//
//Checking program status
//
GLint Program::CheckStatus(GLenum param) const
{
	WriteToLog("Checking shader program status:\n");
	if (!program)
	{
		WriteToLog("ERROR: Program is empty\n");
		return -1;
	}

	GLint status, length;
	GLchar buffer[1024];

	glGetProgramiv(program, param, &status);

	if (status != GL_TRUE)
	{
		glGetProgramInfoLog(program, 1024, &length, buffer);
		WriteToLog("ERROR: Something is wrong with shader program: %s\n", (const char*)buffer);
	}

	OPENGL_CHECK_FOR_ERRORS();

	WriteToLog("OK: Program was checked and no errors were detected\n");

	return status;
}

//
//Checking shader status
//
GLint Shader::CheckStatus(GLenum param)
{
	WriteToLog("Checking shader status:\n");
	if (!shader)
	{
		WriteToLog("ERROR: Shader is empty\n");
		return -1;
	}

	GLint status, length;
	GLchar buffer[1024];

	glGetShaderiv(shader, param, &status);

	if (status != GL_TRUE)
	{
		glGetShaderInfoLog(shader, 1024, &length, buffer);
		WriteToLog("ERROR: Something is wrong with shader: %s\n", (const char*)buffer);
	}

	OPENGL_CHECK_FOR_ERRORS();

	WriteToLog("OK: Shader was checked and no errors were detected\n");

	return status;
}

//
//Attaching shader to program
//
bool Program::AttachShader(const Shader& shader)
{
	if (!program)
		if ((program = glCreateProgram()) == 0)
		{
			WriteToLog("ERROR: Creating shader program fail (%d)\n", glGetError());
		}

	glAttachShader(program, shader.shader);

	OPENGL_CHECK_FOR_ERRORS();

	return true;
}

//
//Linking program
//
bool Program::Link() const
{
	WriteToLog("Linking shader program:\n");
	glLinkProgram(program);

	if(CheckStatus(GL_LINK_STATUS) != GL_TRUE)
	{
		WriteToLog("ERROR: Linking shader program failed (%d)\n", glGetError());
		return false;
	}

	OPENGL_CHECK_FOR_ERRORS();

	WriteToLog("OK: Linking was successfull\n");

	return true;
}

//
//Loading shader from file
//
bool Shader::CreateFromFile(const string& fileName, GLenum typel)
{
	WriteToLog("Loading shader from file: (%s)\n", fileName.c_str());

	uint8_t  *shaderSource;
	uint32_t sourceLength;
	GLuint newshader;

	// create new OpenGL shader
	if ((newshader = glCreateShader(typel)) == 0)
	{
		WriteToLog("ERROR: Can't create shader\n");
		OPENGL_CHECK_FOR_ERRORS();
		return false;
	}

	// load source code
	if (!LoadFile(fileName.c_str(), true, &shaderSource, &sourceLength))
	{
		WriteToLog("ERROR: Can't load file %s\n", fileName);
		OPENGL_CHECK_FOR_ERRORS();
		glDeleteShader(newshader);
		return false;
	}

	// compile shader
	glShaderSource(newshader, 1, (const GLchar**)&shaderSource, (const GLint*)&sourceLength);
	glCompileShader(newshader);

	delete[] shaderSource;

	// check shader compilation status
	GLint status;
	glGetShaderiv(newshader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		WriteToLog("ERROR: Shader compilation failed\n");
		OPENGL_CHECK_FOR_ERRORS();
		glDeleteShader(newshader);
		return false;
	}

	if(shader)
		glDeleteShader(shader);

	WriteToLog("OK: Shader was succesfully loaded\n");

	shader = newshader;
	type = typel;
	OPENGL_CHECK_FOR_ERRORS();
	return true;
}

//
//Validating program
//
bool Program::Validate() const
{
	glValidateProgram(program);
	return (CheckStatus(GL_VALIDATE_STATUS) == GL_TRUE);
}

//
//Setting up the current program
//
void Program::Use() const
{
	glUseProgram(program);
}

