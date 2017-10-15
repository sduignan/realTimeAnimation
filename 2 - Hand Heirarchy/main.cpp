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
#include "text.h"
#include "gl_utils.h" // common opengl functions and small utilities like logs

#define STBI_ASSERT(x)

//My own classes
#include "scene_object.h"
#include "shader.h"
#include "Skeleton.h"

int g_gl_width = 1200;
int g_gl_height = 800;
bool close_window = false;
GLFWwindow* g_window = NULL;

float model_rotation = 0.0f;

// camera matrices. it's easier if they are global
mat4 view_mat;
mat4 proj_mat;
vec3 cam_pos(0.0f, 0.0f, 5.0f);

bool cam_moved = false;
vec3 move(0.0, 0.0, 0.0);
float cam_yaw = 0.0f; // y-rotation in degrees
float cam_pitch = 0.0f;
float cam_roll = 0.0;

float increment = 2.0f;
float trans_increment = 0.25f;

// keep track of some useful vectors that can be used for keyboard movement
vec4 fwd(0.0f, 0.0f, -1.0f, 0.0f);
vec4 rgt(1.0f, 0.0f, 0.0f, 0.0f);
vec4 up(0.0f, 1.0f, 0.0f, 0.0f);

versor q;

ShadowedShader simple_shader;
Shader depth_shader, alpha_shader;

int turn = 1;
int choice = 0;
int num_choices = 3;


GLuint light_depth_fb;
GLuint light_depth_fb_tex;
int light_shadow_size = 1024;

mat4 light_V;
mat4 light_P;

Skeleton Jack;

#define NUM_BONES 20

Bone bones[NUM_BONES];

struct bone_info {
	std::string name;
	int parent_index;
	vec3 init_offset;
};

bone_info bonios[] = {
	{ "HandRoot", -1, vec3(0, 0, 0)},
	{ "thumb0", 0, vec3(0.6948, 0.1529, 0.1527) },
	{ "thumb1", 1, vec3(0.1275, 1.1069, -1.2703) },
	{ "thumb2", 2, vec3(0.1396, 1.8422, -1.4594) },
	{ "index0", 0, vec3(0.4998, 0.3524, 0.0106) },
	{ "index1", 4, vec3(0.6130, 2.0468, 0.0067) },
	{ "index2", 5, vec3(0.7083, 2.9980, 0.0843) },
	{ "index3", 6, vec3(0.7792, 3.5804, 0.2661) },
	{ "middle0", 0, vec3(0.1843, 0.4446, 0.0137) },
	{ "middle1", 8, vec3(0.1370, 2.1477, 0.0062) },
	{ "middle2", 9, vec3(0.1629, 3.2372, 0.0771) },
	{ "middle3", 10, vec3(0.2229, 3.9020, 0.2659) },
	{ "ring0", 0, vec3(-0.0988, 0.4546, 0.0126) },
	{ "ring1", 12, vec3(-0.2749, 2.0842, 0.0060) },
	{ "ring2", 13, vec3(-0.3361, 3.1306, 0.0701) },
	{ "ring3", 14, vec3(-0.3411, 3.7725, 0.2658) },
	{ "pinky0", 0, vec3(-0.3623, 0.4164, 0.0158) },
	{ "pinky1", 16, vec3(-0.6153, 1.7474, 0.0092) },
	{ "pinky2", 17, vec3(-0.7423, 2.5983, 0.0674) },
	{ "pinky3", 18, vec3(-0.7858, 3.1348, 0.2221) }
};

bool pause = false;
int curr_bone_index = 0;

//keyboard control
void My_Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			close_window = true;
			break;
		case GLFW_KEY_LEFT: {
			cam_yaw += increment;
			cam_moved = true;
			versor q_yaw = quat_from_axis_deg(cam_yaw, up.v[0], up.v[1], up.v[2]);
			q = q_yaw * q;
			break;
		}
		case GLFW_KEY_RIGHT: {
			cam_yaw -= increment;
			cam_moved = true;
			versor q_yaw = quat_from_axis_deg(
				cam_yaw, up.v[0], up.v[1], up.v[2]
			);
			q = q_yaw * q;
			break;
		}
		case GLFW_KEY_UP: {
			cam_pitch += increment;
			cam_moved = true;
			versor q_pitch = quat_from_axis_deg(
				cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2]
			);
			q = q_pitch * q;
			break;
		}
		case GLFW_KEY_DOWN: {
			cam_pitch -= increment;
			cam_moved = true;
			versor q_pitch = quat_from_axis_deg(
				cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2]
			);
			q = q_pitch * q;
			break;
		}
		case GLFW_KEY_Z: {
			cam_roll -= increment;
			cam_moved = true;
			versor q_roll = quat_from_axis_deg(
				cam_roll, fwd.v[0], fwd.v[1], fwd.v[2]
			);
			q = q_roll * q;
			break;
		}
		case GLFW_KEY_C: {
			cam_roll -= increment;
			cam_moved = true;
			versor q_roll = quat_from_axis_deg(
				cam_roll, fwd.v[0], fwd.v[1], fwd.v[2]
			);
			q = q_roll * q;
			break;
		}
		case GLFW_KEY_A: {
			move.v[0] -= trans_increment;
			cam_moved = true;
			break;
		}
		case GLFW_KEY_D: {
			move.v[0] += trans_increment;
			cam_moved = true;
			break;
		}
		case GLFW_KEY_W: {
			move.v[2] -= trans_increment;
			cam_moved = true;
			break;
		}
		case GLFW_KEY_S: {
			move.v[2] += trans_increment;
			cam_moved = true;
			break;
		}
		case GLFW_KEY_Q: {
			move.v[1] -= trans_increment;
			cam_moved = true;
			break;
		}
		case GLFW_KEY_E: {
			move.v[1] += trans_increment;
			cam_moved = true;
			break;
		}
		case GLFW_KEY_U: {
			if (pause) {
				versor rotation = quat_from_axis_deg(increment, bones[curr_bone_index].rotation_axis);
				bones[curr_bone_index].local_orientation = quat_to_mat4(rotation)*bones[curr_bone_index].local_orientation;
				Jack.transform(identity_mat4());
				bones[curr_bone_index].rot += increment;
			}
			break;
		}
		case GLFW_KEY_O: {
			if (pause) {
				versor rotation = quat_from_axis_deg(-increment, bones[curr_bone_index].rotation_axis);
				bones[curr_bone_index].local_orientation = quat_to_mat4(rotation)*bones[curr_bone_index].local_orientation;
				Jack.transform(identity_mat4());
				bones[curr_bone_index].rot -= increment;
			}
			break;
		}
		case GLFW_KEY_J: {
			if (pause) {
				Jack.transform(rotate_y_deg(identity_mat4(), -increment));
			}
			break;
		}
		case GLFW_KEY_L: {
			if (pause) {
				Jack.transform(rotate_y_deg(identity_mat4(), increment));
			}
			break;
		}
		case GLFW_KEY_P: {
			Jack.print();
			break;
		}
		case GLFW_KEY_SPACE: {
			curr_bone_index = (curr_bone_index + 1) % NUM_BONES;
			break;
		}
		case GLFW_KEY_ENTER: {
			pause = !pause;
			break;
		}
		}
	}
}

float max_angle = 75;
float current_angle = 0;
float hand_increment = 0.75;
int indices[] = {3, 5, 6, 7, 9, 10, 11, 13, 14, 15, 17, 18, 19};

void moveHand() {
	if ((current_angle < 0 && hand_increment < 0) || (current_angle >= max_angle && hand_increment > 0)) {
		hand_increment = -hand_increment;
	}
	current_angle += hand_increment;

	for (int i = 0; i < 13; i++) {
		versor rotation = quat_from_axis_deg(hand_increment, bones[indices[i]].rotation_axis);

		bones[indices[i]].local_orientation = quat_to_mat4(rotation)*bones[indices[i]].local_orientation;
		bones[indices[i]].rot = current_angle;
	}

	Jack.transform(identity_mat4());
}


void cast_shadows(Skeleton Jack) {
	//bind framebuffer that renders to texture
	glBindFramebuffer(GL_FRAMEBUFFER, light_depth_fb);
	//set viewpor to correct size
	glViewport(0, 0, light_shadow_size, light_shadow_size);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// no need to clear the colour buffer
	glClear(GL_DEPTH_BUFFER_BIT);
	// bind out shadow-casting shader from the previous section
	glUseProgram(depth_shader.id);
	// send in the view and projection matrices from the light
	glUniformMatrix4fv(depth_shader.V_loc, 1, GL_FALSE, light_V.m);
	glUniformMatrix4fv(depth_shader.P_loc, 1, GL_FALSE, light_P.m);
	
	Jack.draw(depth_shader.M_loc);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


int main() {
	//START OPENGL
	restart_gl_log();
	// start GL context and O/S window using the GLFW helper library
	start_gl();

	// Tell the window where to find its key callback function
	glfwSetKeyCallback(g_window, My_Key_Callback);
	
	for (int i = 0; i < NUM_BONES; i++) {
		std::string mesh_loc = "Meshes/" + bonios[i].name + ".obj";
		bones[i].load_mesh(mesh_loc.c_str());
		bones[i].name = bonios[i].name;
		if (bonios[i].parent_index >= 0) {
			bones[bonios[i].parent_index].add_child(&bones[i]);
			if (i == 2) {
				bones[i].set_offset(bonios[i].init_offset - vec3(0.0916, 0.1529, -0.7055));
			}
			else {
				bones[i].set_offset(bonios[i].init_offset - bonios[bonios[i].parent_index].init_offset);
			}
		}
		else {
			Jack.root = &bones[i];
			bones[i].set_offset(bonios[i].init_offset);
		}
		bones[i].set_axis();
	}

	bones[1].local_orientation = rotate_y_deg(identity_mat4(), -95)*bones[1].local_orientation;
	
	Jack.bone_count - NUM_BONES;
	Jack.transform(translate(rotate_x_deg(identity_mat4(), 90), vec3(0, 0, 0)));

	// Depth Map Shader
	depth_shader.id = create_programme_from_files("Shaders/depth.vert", "Shaders/depth.frag");
	glUseProgram(simple_shader.id);
	//get locations of M, V, P matrices
	depth_shader.M_loc = glGetUniformLocation(depth_shader.id, "M");
	assert(depth_shader.M_loc > -1);
	depth_shader.V_loc = glGetUniformLocation(depth_shader.id, "V");
	assert(depth_shader.V_loc > -1);
	depth_shader.P_loc = glGetUniformLocation(depth_shader.id, "P");
	assert(depth_shader.P_loc > -1);

	//Hightlighting Shader
	alpha_shader.id = create_programme_from_files("Shaders/alpha.vert", "Shaders/alpha.frag");
	glUseProgram(alpha_shader.id);
	//get locations of M, V, P matrices
	alpha_shader.M_loc = glGetUniformLocation(alpha_shader.id, "M");
	assert(alpha_shader.M_loc > -1);
	alpha_shader.V_loc = glGetUniformLocation(alpha_shader.id, "V");
	assert(alpha_shader.V_loc > -1);
	alpha_shader.P_loc = glGetUniformLocation(alpha_shader.id, "P");
	assert(alpha_shader.P_loc > -1);
	

	// Main Shader
	simple_shader.id = create_programme_from_files("Shaders/light.vert", "Shaders/light.frag");
	glUseProgram(simple_shader.id);
	//get locations of M, V, P matrices
	simple_shader.M_loc = glGetUniformLocation(simple_shader.id, "M");
	assert(simple_shader.M_loc > -1);
	simple_shader.V_loc = glGetUniformLocation(simple_shader.id, "V");
	assert(simple_shader.V_loc > -1);
	simple_shader.P_loc = glGetUniformLocation(simple_shader.id, "P");
	assert(simple_shader.P_loc > -1);
	simple_shader.caster_V_loc = glGetUniformLocation(simple_shader.id, "caster_V");
	//assert(simple_shader.caster_V_loc > -1);
	simple_shader.caster_P_loc = glGetUniformLocation(simple_shader.id, "caster_P");
	//assert(simple_shader.caster_P_loc > -1);

	GLuint texLoc = glGetUniformLocation(simple_shader.id, "depth_map");
	glUniform1i(texLoc, 0);
	
	//Set up framebuffer that renders to texture for light-depth values
	{
		// create framebuffer
		glGenFramebuffers(1, &light_depth_fb);
		glBindFramebuffer(GL_FRAMEBUFFER, light_depth_fb);

		// create texture for framebuffer
		glGenTextures(1, &light_depth_fb_tex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, light_depth_fb_tex);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_DEPTH_COMPONENT,
			light_shadow_size,
			light_shadow_size,
			0,
			GL_DEPTH_COMPONENT,
			GL_UNSIGNED_BYTE,
			NULL
		);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// clamp to edge. Scene positioned so that all shadow casting objects fall within camera view
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach depth texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light_depth_fb_tex, 0);

		// tell framebuffer not to use any colour drawing outputs
		GLenum draw_bufs[] = { GL_NONE };
		glDrawBuffers(1, draw_bufs);

		//No buffer to read from for drawing at this time
		glReadBuffer(GL_NONE);

		// bind default framebuffer again
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	//CREATE CAMERA
	// input variables
	float near = 0.1f; // clipping plane
	float far = 100.0f; // clipping plane
	float fovy = 67.0f; // 67 degrees
	float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
	proj_mat = perspective(fovy, aspect, near, far);

	mat4 T = translate(
		identity_mat4(), vec3(0, -1, -5)
	);
	mat4 R = rotate_y_deg(identity_mat4(), -0.0);
	q = quat_from_axis_deg(-0.0, 0.0f, 1.0f, 0.0f);
	view_mat = R*T;


	//SET RENDERING DEFAULTS
	glUseProgram(simple_shader.id);
	glUniformMatrix4fv(simple_shader.P_loc, 1, GL_FALSE, proj_mat.m);
	glUniformMatrix4fv(simple_shader.V_loc, 1, GL_FALSE, view_mat.m);
	glUniformMatrix4fv(simple_shader.M_loc, 1, GL_FALSE, identity_mat4().m);


	glUseProgram(alpha_shader.id);
	glUniformMatrix4fv(alpha_shader.P_loc, 1, GL_FALSE, proj_mat.m);
	glUniformMatrix4fv(alpha_shader.V_loc, 1, GL_FALSE, view_mat.m);
	glUniformMatrix4fv(alpha_shader.M_loc, 1, GL_FALSE, identity_mat4().m);

	//SHADOW CASTER VARIABLES
	vec3 light_pos(0.0, 12.0, 0.0);
	vec3 light_target(0.0, 0.0, 0.0);
	vec3 up_dir(0.0, 0.0, -1.0);
	light_V = look_at(light_pos, light_target, up_dir);

	float light_near = 0.5f;
	float light_far = 15.0f;
	float light_fov = 150.0f;
	float light_aspect = 1.0f;
	light_P = perspective(light_fov, light_aspect, light_near, light_far);

	glUseProgram(simple_shader.id);
	glUniformMatrix4fv(simple_shader.caster_V_loc, 1, GL_FALSE, light_V.m);
	glUniformMatrix4fv(simple_shader.caster_P_loc, 1, GL_FALSE, light_P.m);


	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glDisable(GL_CULL_FACE); // cull face
	glFrontFace(GL_CCW); // set counter-clock-wise vertex order to mean the front
	glClearColor(0.0, 0.0, 0.0, 1.0); // black background for bleakness
	glViewport(0, 0, g_gl_width, g_gl_height);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	Jack.transform(identity_mat4());

	//MAIN LOOP
	while (!glfwWindowShouldClose(g_window)) {

		//cast shadows
		cast_shadows(Jack);

		// wipe the drawing surface clear
		glViewport(0, 0, g_gl_width, g_gl_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(simple_shader.id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, light_depth_fb_tex);

		glUniformMatrix4fv(simple_shader.V_loc, 1, GL_FALSE, view_mat.m);
		Jack.draw(simple_shader.M_loc);

		if (pause) {
			glUseProgram(alpha_shader.id);
			glUniformMatrix4fv(alpha_shader.V_loc, 1, GL_FALSE, view_mat.m);
			glUniformMatrix4fv(alpha_shader.M_loc, 1, GL_FALSE, bones[curr_bone_index].global_transformation.m);
			glBindVertexArray(bones[curr_bone_index].vao);
			glDrawArrays(GL_TRIANGLES, 0, bones[curr_bone_index].point_count);
		}
		else {
			moveHand();
		}

		// update view matrix
		if (cam_moved) {
			// re-calculate local axes so can move fwd in dir cam is pointing
			R = quat_to_mat4(q);
			fwd = R * vec4(0.0, 0.0, -1.0, 0.0);
			rgt = R * vec4(1.0, 0.0, 0.0, 0.0);
			up = R * vec4(0.0, 1.0, 0.0, 0.0);

			cam_pos = cam_pos + vec3(fwd) * -move.v[2];
			cam_pos = cam_pos + vec3(up) * move.v[1];
			cam_pos = cam_pos + vec3(rgt) * move.v[0];
			mat4 T = translate(identity_mat4(), vec3(cam_pos));

			view_mat = inverse(R) * inverse(T);
		}

		cam_moved = false;
		cam_yaw = 0.0f;
		cam_pitch = 0.0f;
		cam_roll = 0.0;
		move = vec3(0.0, 0.0, 0.0);

		glfwPollEvents();
		glfwSwapBuffers(g_window);
		if (close_window) {
			glfwDestroyWindow(g_window);
		}
	}
}