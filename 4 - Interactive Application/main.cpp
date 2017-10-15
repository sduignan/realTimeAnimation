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
#include <time.h>
#include <algorithm>

//3rd party code
#include "Antons_maths_funcs.h"
#include "obj_parser.h"
#include "text.h"
#include "gl_utils.h" // common opengl functions and small utilities like logs

#define STBI_ASSERT(x)

//My own classes
#include "scene_object.h"
#include "cube_map.h"
#include "shader.h"
#include "Skeleton.h"
#include "BoneList.h"
#include "CCD.h"
#include "Spline.h"
#include "Grabbed.h"

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
Shader depth_shader, tex_shader, point_shader;

int turn = 1;
int choice = 0;
int num_choices = 3;


GLuint light_depth_fb;
GLuint light_depth_fb_tex;
int light_shadow_size = 1024;

mat4 light_V;
mat4 light_P;

cube_map sky_cube;
Skeleton Jack;
Skeleton Snitch;

//#define NUM_BONES 20

Bone bones[NUM_BONES];
Bone snitchBits[3];

vec3 target;

bool pause = false;
int curr_bone_index = 0;

double x, y;
bool moving = false;
float mouse_inc = 0.05f;

Spline spline[4];


int grabCount = 0;
bool grabbing = false;
bool caught = false;

void grab(Bone* bone) {
	if (grabCount == 75) {
		grabbing = false;
		grabCount = 0;
		return;
	}
	
	if (grabCount > 50 && caught) {
		return;
	}

	grabCount++;

	if (grabCount < 10) {
		vec3 hand = bone->children[2]->children[0]->world_coords();
		vec3 t = Snitch.root->world_coords();

		if (abs(hand.v[0] - t.v[0]) < 0.08 && abs(hand.v[1] - t.v[1]) < 0.08) {
			caught = true;
		}
		else {
			printf("Difs:\n\tX: %f, Y: %f\n", abs(hand.v[0] - t.v[0]), abs(hand.v[1] - t.v[1]));
		}
	}

	if (grabCount > 10 && grabCount < 51) {
		return;
	}

	

	bone->grab(grabCount);
	Jack.transform(identity_mat4());
}

float flapDeg = 0;
float flapInc = 11.0f;
bool flapping = true;
float maxFlapAngle = 45.0f;

float bounds[] = { -1.40 , -0.7, 1.70, 3.10 };
int x_range = (int)((bounds[1] - bounds[0]) * 100);
int y_range = (int)((bounds[3] - bounds[2]) * 100);

int twitchFrames = 0;
vec3 journey;
vec3 backstep;

void twitchSnitch() {
	if (twitchFrames < 20) {
		twitchFrames++;
		return;
	}

	if (twitchFrames == 20) {
		if (rand() % 10 > 2) {
			int x = rand() % x_range;
			int y = rand() % y_range;

			journey = vec3(((float)x) / 100.0 + bounds[0], ((float)y) / 100.0 + bounds[2], Snitch.root->init_offset.v[2]) - Snitch.root->init_offset;
			backstep = normalise(journey)*-0.01;
			journey += backstep*-10;
			journey = journey*0.1;
			Snitch.root->init_offset.v[1];
			twitchFrames++;
		}
		return;
	}

	if (twitchFrames <= 25) {
		Snitch.root->init_offset += backstep;
		twitchFrames++;
		return;
	}
	if (twitchFrames <= 35) {
		Snitch.root->init_offset += journey;
		twitchFrames++;
		return;
	}
	if (twitchFrames <= 40) {
		Snitch.root->init_offset += backstep;
		twitchFrames++;
		return;
	}
	twitchFrames = 0;
}

void flap() {
	if ((flapDeg <= -maxFlapAngle && flapInc < 0) || (flapDeg >= maxFlapAngle && flapInc > 0)) {
		flapInc = -flapInc;
	}
	flapDeg += flapInc;
	Snitch.root->children[0]->local_orientation = rotate_z_deg(Snitch.root->children[0]->local_orientation, flapInc);
	Snitch.root->children[1]->local_orientation = rotate_z_deg(Snitch.root->children[1]->local_orientation, -flapInc);

	Snitch.transform(identity_mat4());
}

//Mouse movement control
void My_Mouse_Callback(GLFWwindow* window, double xpos, double ypos) {
	if(!pause) {
		//Translation
		glfwGetCursorPos(window, &x, &y);

		float horizontal_trans = 0.1*mouse_inc*(x - (double)g_gl_width / 2.0);
		float vertical_trans = -0.1*mouse_inc*(y - (double)g_gl_height / 2.0);

		target.v[0] += horizontal_trans;
		target.v[0] = std::max(bounds[0], std::min(target.v[0], bounds[1]));
		target.v[1] += vertical_trans;
		target.v[1] = std::max(bounds[2], std::min(target.v[1], bounds[3]));
		glfwSetCursorPos(window, (double)g_gl_width / 2.0, (double)g_gl_height / 2.0);
	}
}

//Mouse button control
void Mouse_Button_Callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
		grabbing = true;
	}
}

float scroll_inc = 0.01;

//Scroll Control
void Scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	if (!pause) {
		target.v[2] -= trans_increment*yoffset;
	}
}

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
				bones[curr_bone_index].local_orientation = rotate_z_deg(bones[curr_bone_index].local_orientation, increment);
				Jack.transform(identity_mat4());
			}
			break;
		}
		case GLFW_KEY_O: {
			if (pause) {
				bones[curr_bone_index].local_orientation = rotate_z_deg(bones[curr_bone_index].local_orientation, -increment);
				Jack.transform(identity_mat4());
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
			printf("Target: ");
			print(target);
			break;
		}
		case GLFW_KEY_SPACE: {
			caught = false;
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
		bones[indices[i]].local_orientation = rotate_z_deg(bones[indices[i]].local_orientation, hand_increment);
	}

	Jack.transform(identity_mat4());
}


int main() {
	//START OPENGL
	restart_gl_log();
	// start GL context and O/S window using the GLFW helper library
	start_gl();

	// Tell the window where to find its key callback function
	glfwSetKeyCallback(g_window, My_Key_Callback);

	// Tell window where to find the Mouse callback function
	glfwSetCursorPosCallback(g_window, My_Mouse_Callback);

	//Cursor button callback:
	glfwSetMouseButtonCallback(g_window, Mouse_Button_Callback);
	glfwSetInputMode(g_window, GLFW_STICKY_MOUSE_BUTTONS, 1);
	glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	//Scroll Control
	glfwSetScrollCallback(g_window, Scroll_callback);

	//CUBE MAP
	sky_cube.setup(FRONT, BACK, TOP, BOTTOM, LEFT, RIGHT);

	//Grabby hand
	setGrabMapping();

	srand(time(NULL));
	
	for (int i = 0; i < NUM_BONES; i++) {
		std::string mesh_loc = mesh_dir + bone_list[i].name + ".obj";
		bones[i].load_mesh(mesh_loc.c_str());
		bones[i].name = bone_list[i].name;
		bones[i].set_offset(bone_list[i].init_offset);
		bones[i].local_orientation = rotate_x_deg(rotate_y_deg(rotate_z_deg(identity_mat4(), bone_list[i].z_rot), bone_list[i].y_rot), bone_list[i].x_rot);
		if (bone_list[i].parent_index >= 0) {
			bones[bone_list[i].parent_index].add_child(&bones[i]);
		}
		else {
			Jack.root = &bones[i];
		}
	}


	Jack.bone_count = NUM_BONES;

	//Snitch Bizniss
	for (int i = 0; i < NUM_SNITCHBITS; i++) {
		std::string mesh_loc = snitch_mesh_dir + snitchbit_list[i].name + ".obj";
		snitchBits[i].load_mesh(mesh_loc.c_str());
		snitchBits[i].name = snitchbit_list[i].name;
		snitchBits[i].set_offset(snitchbit_list[i].init_offset);
		snitchBits[i].load_tex(snitchTexes[i].c_str());
		snitchBits[i].local_orientation = rotate_x_deg(rotate_y_deg(rotate_z_deg(identity_mat4(), snitchbit_list[i].z_rot), snitchbit_list[i].y_rot), snitchbit_list[i].x_rot);
		if (snitchbit_list[i].parent_index >= 0) {
			snitchBits[snitchbit_list[i].parent_index].add_child(&snitchBits[i]);
		}
		else {
			Snitch.root = &snitchBits[i];
		}
	}

	Snitch.bone_count = NUM_SNITCHBITS;


	// Create yoke to draw point:
	GLuint point_vao, point_vbo;
	glGenVertexArrays(1, &point_vao);
	glBindVertexArray(point_vao);
	glGenBuffers(1, &point_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, point_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3, target.v, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//Texturing Shader
	tex_shader.id = create_programme_from_files("Shaders/tex.vert", "Shaders/tex.frag");
	glUseProgram(tex_shader.id);
	//get locations of M, V, P matrices
	tex_shader.M_loc = glGetUniformLocation(tex_shader.id, "M");
	assert(tex_shader.M_loc > -1);
	tex_shader.V_loc = glGetUniformLocation(tex_shader.id, "V");
	assert(tex_shader.V_loc > -1);
	tex_shader.P_loc = glGetUniformLocation(tex_shader.id, "P");
	assert(tex_shader.P_loc > -1);


	//Point Shader
	point_shader.id = create_programme_from_files("Shaders/string.vert", "Shaders/string.frag");
	glUseProgram(point_shader.id);
	//get locations of V, P matrices
	point_shader.V_loc = glGetUniformLocation(point_shader.id, "V");
	assert(point_shader.V_loc > -1);
	point_shader.P_loc = glGetUniformLocation(point_shader.id, "P");
	assert(point_shader.P_loc > -1);
	

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
	
	// cube-map shaders
	sky_cube.shader = create_programme_from_files("Shaders/sky_cube.vert", "Shaders/sky_cube.frag");
	glUseProgram(sky_cube.shader);
	// note that this view matrix should NOT contain camera translation.
	sky_cube.proj_loc = glGetUniformLocation(sky_cube.shader, "P");
	sky_cube.view_loc = glGetUniformLocation(sky_cube.shader, "V");

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


	glUseProgram(tex_shader.id);
	glUniformMatrix4fv(tex_shader.P_loc, 1, GL_FALSE, proj_mat.m);
	glUniformMatrix4fv(tex_shader.V_loc, 1, GL_FALSE, view_mat.m);
	glUniformMatrix4fv(tex_shader.M_loc, 1, GL_FALSE, identity_mat4().m);

	glUseProgram(point_shader.id);
	glUniformMatrix4fv(point_shader.P_loc, 1, GL_FALSE, proj_mat.m);
	glUniformMatrix4fv(point_shader.V_loc, 1, GL_FALSE, view_mat.m);
	
	sky_cube.proj = proj_mat;
	sky_cube.view = R;
	sky_cube.update_mats(true, true);

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
	
	mat4 cube_view_mat;
	mat4 sea_view_mat;

	Jack.transform(identity_mat4());
	std::string topName = "core";
	Bone* endEffector = Jack.root->children[0]->children[0]->children[0];
	//Bone* endEffector = Jack.root->children[0]->children[0]->children[0]->children[1]->children[0]->children[0]->children[0];
	target = endEffector->world_coords();
	int its = 0;
	float inc = 0.05;

	Snitch.transform(identity_mat4());

	vec3 points[4][4] = { 
		{ vec3(0.1, 1, 0), vec3(-0.3, 0, -0.4), vec3(-0.2, -0.6, 0), vec3(-0.2, 0, 0.4) },
		{ vec3(-0.3, 0, -0.4), vec3(-0.2, -0.6, 0), vec3(-0.2, 0, 0.4), vec3(0.1, 1, 0) },
		{ vec3(-0.2, -0.6, 0), vec3(-0.2, 0, 0.4), vec3(0.1, 1, 0), vec3(-0.3, 0, -0.4) },
		{ vec3(-0.2, 0, 0.4), vec3(0.1, 1, 0), vec3(-0.3, 0, -0.4), vec3(-0.2, -0.6, 0) } };

	vec3 offset = target + vec3(0.2, 0, 0);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			points[i][j] += offset;
		}
	}
	spline[0].setPoints(points[1]);
	spline[1].setPoints(points[2]);
	spline[2].setPoints(points[3]);
	spline[3].setPoints(points[0]);

	float spline_inc = 1.0 / 90.0;
	float spline_t = 0.0;
	int spline_choice;


	float ps[4*3];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			ps[i * 3 + j] = points[0][i].v[j];
		}
	}

	// Create yoke to draw point:
	GLuint spline_vao, spline_vbo;
	glGenVertexArrays(1, &spline_vao);
	glBindVertexArray(spline_vao);
	glGenBuffers(1, &spline_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, spline_vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float) * 3, ps, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);


	glPointSize(6);

	Snitch.root->local_orientation = scale(Snitch.root->local_orientation, vec3(0.1, 0.1, 0.1));
	Snitch.root->init_offset += vec3(0.0, 0.0, 0.3);

	twitchSnitch();
	Snitch.transform(identity_mat4());
	
	//MAIN LOOP
	while (!glfwWindowShouldClose(g_window)) {
		
		// wipe the drawing surface clear
		glViewport(0, 0, g_gl_width, g_gl_height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(simple_shader.id);
		glUniformMatrix4fv(simple_shader.V_loc, 1, GL_FALSE, view_mat.m);
		Jack.draw(simple_shader.M_loc);

		glUseProgram(tex_shader.id);
		glUniformMatrix4fv(tex_shader.V_loc, 1, GL_FALSE, view_mat.m);
		Snitch.draw(tex_shader.M_loc);

		CCD(Jack, target, endEffector, topName);
		
		if (grabbing) {
			grab(endEffector);
		}
		if (!caught) {
			flap();
			twitchSnitch();
		}
		else {
			Snitch.root->init_offset = endEffector->children[2]->children[0]->world_coords();
			Snitch.transform(identity_mat4());
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

			// cube-map view matrix has rotation, but not translation
			sky_cube.view = inverse(R);
			sky_cube.update_mats(false, true);
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