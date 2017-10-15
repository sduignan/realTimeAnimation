#pragma once
#include "Antons_maths_funcs.h"
#include "scene_object.h"
#include "shader.h"

class Plane {
public:
	scene_object plane_model;
	scene_object propellor;
	float euler_x, euler_y, euler_z = 0.0f;
	float heading = 0.0f;
	vec3 location = vec3(0.0, 0.0, -5.0);
	versor q;
	vec4 fwd = vec4(0.0f, 0.0f, -1.0f, 0.0f);
	vec4 rgt = vec4(1.0f, 0.0f, 0.0f, 0.0f);
	vec4 up = vec4(0.0f, 1.0f, 0.0f, 0.0f);
	vec3 move;
	
	mat4 model_mat;
	Shader shader;

	mat4 prop_rot_mat = mat4(0.93969262078, 0.34202014332, 0, 0, -0.34202014332, 0.93969262078, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

	Plane();

	void setup(char* mesh_file_path, char* tex_file_path, char* prop_file_path, char* prop_tex_file_path, Shader shad);
	
	void yaw(float scalar);
	void pitch(float scalar);
	void roll(float scalar);

	void q_yaw(float scalar);
	void q_pitch(float scalar);
	void q_roll(float scalar);

	void draw();
	void q_draw();
	void fpp_draw();

	mat4 get_fpp_cube_view_mat();
};

