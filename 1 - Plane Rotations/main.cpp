#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>

//3rd party code
#include "Antons_maths_funcs.h"
#include "obj_parser.h"
#include "gl_utils.h" // common opengl functions and small utilities like logs

#define STBI_ASSERT(x)

//My own classes
#include "scene_object.h"
#include "cube_map.h"
#include "shader.h"
#include "plane.h"

#define FRONT "cube_map/negz.png"
#define BACK "cube_map/posz.png"
#define TOP "cube_map/posy.png"
#define BOTTOM "cube_map/negy.png"
#define LEFT "cube_map/negx.png"
#define RIGHT "cube_map/posx.png"

int g_gl_width = 1200;
int g_gl_height = 800;
bool close_window = false;
GLFWwindow* g_window = NULL;

float model_rotation = 0.0f;

mat4 view_mat;
mat4 proj_mat;

float increment = 2.0f;
float trans_increment = 0.25f;

versor q;

Shader simple_shader;
Shader cube_shader;

int turn = 1;
int choice = 0;
int num_choices = 3;


Plane actual_plane;
cube_map sky_cube;

//keyboard control
void My_Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			close_window = true;
			break;
		case GLFW_KEY_LEFT: {
			if (choice == 0) {
				actual_plane.yaw(increment);
			}
			else {
				actual_plane.q_yaw(increment);
			}
			break;
		}
		case GLFW_KEY_RIGHT: {
			if (choice == 0) {
				actual_plane.yaw(-increment);
			}
			else {
				actual_plane.q_yaw(-increment);
			}
			break;
		}
		case GLFW_KEY_UP: {
			if (choice == 0) {
				actual_plane.pitch(increment);
			}
			else {
				actual_plane.q_pitch(increment);
			}
			break;
		}
		case GLFW_KEY_DOWN: {
			if (choice == 0) {
				actual_plane.pitch(-increment);
			}
			else {
				actual_plane.q_pitch(-increment);
			}
			break;
		}
		case GLFW_KEY_Z: {
			if (choice == 0) {
				actual_plane.roll(increment);
			}
			else {
				actual_plane.q_roll(increment);			
			}
			break;
		}
		case GLFW_KEY_C: {
			if (choice == 0) {
				actual_plane.roll(-increment);
			}
			else {
				actual_plane.q_roll(-increment);
			}
			break;
		}
		case GLFW_KEY_A: {
			if (choice == 0) {
				actual_plane.location.v[0] -= trans_increment;
			}
			else {
				actual_plane.move.v[0] -= trans_increment;
			}
			break;
		}
		case GLFW_KEY_D: {
			if (choice == 0) {
				actual_plane.location.v[0] += trans_increment;
			}
			else {
				actual_plane.move.v[0] += trans_increment;
			}
			break;
		}
		case GLFW_KEY_W: {
			if (choice == 0) {
				actual_plane.location.v[2] -= trans_increment;
			}
			else {
				actual_plane.move.v[2] -= trans_increment;
			}
			break;
		}
		case GLFW_KEY_S: {
			if (choice == 0) {
				actual_plane.location.v[2] += trans_increment;
			}
			else {
				actual_plane.move.v[2] += trans_increment;
			}
			break;
		}
		case GLFW_KEY_Q: {
			if (choice == 0) {
				actual_plane.location.v[1] -= trans_increment;
			}
			else {
				actual_plane.move.v[1] -= trans_increment;
			}
			break;
		}
		case GLFW_KEY_E: {
			if (choice == 0) {
				actual_plane.location.v[1] += trans_increment;
			}
			else {
				actual_plane.move.v[1] += trans_increment;
			}
			break;
		}
		case GLFW_KEY_SPACE: {
			choice = (choice + 1) % num_choices;
			if (choice == 0) {
				sky_cube.view = identity_mat4();
				sky_cube.update_mats(false, true);
			}
			break;
		}
		case GLFW_KEY_ENTER: {
			break;
		}
		}
	}
}

int main() {
	//START OPENGL
	restart_gl_log();
	// start GL context and O/S window using the GLFW helper library
	start_gl();

	// Tell the window where to find its key callback function
	glfwSetKeyCallback(g_window, My_Key_Callback);

	//CUBE MAP
	sky_cube.setup(FRONT, BACK, TOP, BOTTOM, LEFT, RIGHT);
	

	// Plane Shader
	simple_shader.id = create_programme_from_files("Shaders/tex.vert", "Shaders/tex.frag");
	glUseProgram(simple_shader.id);
	//get locations of M, V, P matrices
	simple_shader.M_loc = glGetUniformLocation(simple_shader.id, "M");
	assert(simple_shader.M_loc > -1);
	simple_shader.V_loc = glGetUniformLocation(simple_shader.id, "V");
	assert(simple_shader.V_loc > -1);
	simple_shader.P_loc = glGetUniformLocation(simple_shader.id, "P");
	assert(simple_shader.P_loc > -1);

	// Cube Shader
	cube_shader.id = create_programme_from_files("Shaders/alpha.vert", "Shaders/alpha.frag");
	glUseProgram(cube_shader.id);
	//get locations of M, V, P matrices
	cube_shader.M_loc = glGetUniformLocation(cube_shader.id, "M");
	assert(cube_shader.M_loc > -1);
	cube_shader.V_loc = glGetUniformLocation(cube_shader.id, "V");
	assert(cube_shader.V_loc > -1);
	cube_shader.P_loc = glGetUniformLocation(cube_shader.id, "P");
	assert(cube_shader.P_loc > -1);

	actual_plane.setup("Meshes/plane/plane.obj", "Textures/tex.png", "Meshes/plane/prop.obj", "Textures/prop.png", simple_shader);

	// cube-map shaders
	sky_cube.shader = create_programme_from_files("Shaders/sky_cube.vert", "Shaders/sky_cube.frag");
	glUseProgram(sky_cube.shader);
	// note that this view matrix should NOT contain camera translation.
	sky_cube.proj_loc = glGetUniformLocation(sky_cube.shader, "P");
	sky_cube.view_loc = glGetUniformLocation(sky_cube.shader, "V");


	//CREATE CAMERA
	// input variables
	float near = 0.1f; // clipping plane
	float far = 100.0f; // clipping plane
	float fovy = 67.0f; // 67 degrees
	float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
	proj_mat = perspective(fovy, aspect, near, far);

	mat4 T = translate(
		identity_mat4(), vec3(0, 0, -5)
	);
	mat4 R = rotate_y_deg(identity_mat4(), -0.0);
	q = quat_from_axis_deg(-0.0, 0.0f, 1.0f, 0.0f);
	view_mat = R*T;


	//SET RENDERING DEFAULTS
	glUseProgram(simple_shader.id);
	glUniformMatrix4fv(simple_shader.P_loc, 1, GL_FALSE, proj_mat.m);
	glUniformMatrix4fv(simple_shader.V_loc, 1, GL_FALSE, view_mat.m);
	
	glUseProgram(cube_shader.id);
	glUniformMatrix4fv(cube_shader.P_loc, 1, GL_FALSE, proj_mat.m);
	glUniformMatrix4fv(cube_shader.V_loc, 1, GL_FALSE, view_mat.m);

	sky_cube.proj = proj_mat;
	sky_cube.view = R;
	sky_cube.update_mats(true, true);

	mat4 ball_model_mat = scale(identity_mat4(), vec3(1.5, 1.5, 1.5));

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	//glEnable(GL_CULL_FACE); // cull face
	//glCullFace(GL_BACK); // cull back face
	glDisable(GL_CULL_FACE); // cull face
	glFrontFace(GL_CCW); // set counter-clock-wise vertex order to mean the front
	glClearColor(0.2, 0.2, 0.2, 1.0); // grey background to help spot mistakes
	glViewport(0, 0, g_gl_width, g_gl_height);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	mat4 cube_view_mat;
	mat4 sea_view_mat;

	//MAIN LOOP
	while (!glfwWindowShouldClose(g_window)) {
		// wipe the drawing surface clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (choice == 2) {
			sky_cube.view = actual_plane.get_fpp_cube_view_mat();
			sky_cube.update_mats(false, true);
		}
		// render a sky-box using the cube-map texture
		sky_cube.draw();

		if (choice == 0) {
			actual_plane.draw();
		}
		else if (choice == 1) {
			actual_plane.q_draw();
		}
		else {
			actual_plane.fpp_draw();
		}

		glfwPollEvents();
		glfwSwapBuffers(g_window);
		if (close_window) {
			glfwDestroyWindow(g_window);
		}
	}
}