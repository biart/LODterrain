#include "Common.h"

/*
	Shader and Program classes
	Allows you to work with GLSL shaders
*/

#ifndef SHADER_H
#define SHADER_H

class Shader;  //Class for OpenGL shader
class Program; //Class for OpenGL shader program

class Shader
{

friend class Program;

public:
	//Constructor and destructor
	Shader(void);
	~Shader(void);

	//Load shader from file
	bool CreateFromFile(const string& fileName, GLenum type);

	//Check shader status
	GLint CheckStatus(GLenum param);

private:
	GLenum type;
	GLuint shader;
};

class Program
{
public:
	//Constructor and destructor
	Program(void);
	~Program(void);

	//Check program status
	GLint CheckStatus(GLenum param) const;

	//Attach shader to the program
	bool AttachShader(const Shader& pShader);

	bool Link(void) const;
	bool Validate(void) const;
	void Use(void) const;

	GLuint program;

private:
	bool active;
};

#endif // SHADER_H