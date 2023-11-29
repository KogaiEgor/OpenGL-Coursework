#pragma once
#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<cerrno>
#include <GL/glew.h>


std::string get_file_contents(const char* filename);

class Skyboxshader
{
public:
	// Reference ID of the Shader Program
	GLuint ID;
	// Constructor that build the Shader Program from 2 different shaders
	Skyboxshader(const char* vertexFile, const char* fragmentFile);

	// Activates the Shader Program
	void Activate();
	// Deletes the Shader Program
	void Delete();
private:
	// Checks if the different Shaders have compiled properly
	void compileErrors(unsigned int shader, const char* type);
};


