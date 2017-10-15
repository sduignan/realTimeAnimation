#pragma once
#include <GL/glew.h>

class Shader {
public:
	GLuint id;
	int M_loc;
	int V_loc;
	int P_loc;
};