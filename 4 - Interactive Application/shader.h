#pragma once
#include <GL/glew.h>

class Shader {
public:
	GLuint id;
	int M_loc;
	int V_loc;
	int P_loc;
};

class ShadowedShader : public Shader {
public:
	int caster_P_loc;
	int caster_V_loc;
};