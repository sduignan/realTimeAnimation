#pragma once

#include "Antons_maths_funcs.h"
#include <string>

#define NUM_BONES 23
#define NUM_SNITCHBITS 3

std::string mesh_dir = "Meshes/BoneByBone/";
std::string snitch_mesh_dir = "Meshes/Snitch/";

struct bone_info {
	std::string name;
	int parent_index;
	vec3 init_offset;
	float x_rot;
	float y_rot;
	float z_rot;
};

bone_info bone_list[] = {
	{ "core", 				-1,		vec3( 0.00662,  1.61625, -0.00310), 0, 0, 0 },
	{ "humorus", 			 0,		vec3(-0.22794,  0.53516, -0.00015), -77.5, 0, 0 },
	{ "ulna", 				 1,		vec3(-0.48599,  0.02443,  0.01494), 0, 0, 9.5 },
	{ "wrist_bits", 		 2,		vec3(-0.35570,  0.01112, -0.01552), 0, 0, 0 },
	{ "thumb_metacarpal",	 3,		vec3(-0.04585,  0.00048,  0.02214), 80, 33.5, 0 },
	{ "thumb_p_phalange",	 4,		vec3(-0.07606,  0.00346, -0.00027), 0, 0, 0 },
	{ "thumb_d_phalange", 	 5,		vec3(-0.05311,  0.00085, -0.00113), 0, 0, 0 },
	{ "index_metacarpal", 	 3,		vec3(-0.05470,  0.01073,  0.01238), 0, 7, 0 },
	{ "index_p_phalange", 	 7,		vec3(-0.11149, -0.00530,  0.00093), 0, 0, 0 },
	{ "index_m_phalange", 	 8,		vec3(-0.07357,  0.00126,  0.00048), 0, 0, 0 },
	{ "index_d_phalange", 	 9,		vec3(-0.04228, -0.00146,  0.00131), 0, 0, 0 },
	{ "middle_metacarpal",	 3,		vec3(-0.05274,  0.01096, -0.00423), 0, 0, 0 },
	{ "middle_p_phalange", 	 11, 	vec3(-0.11327, -0.00137, -0.00546), 0, 0, 0 },
	{ "middle_m_phalange", 	 12, 	vec3(-0.08326, -0.00071,  0.00100), 0, 0, 0 },
	{ "middle_d_phalange", 	 13, 	vec3(-0.04630,  0.00048, -0.00239), 0, 0, 0 },
	{ "ring_metacarpal", 	 3, 	vec3(-0.05510,  0.01668, -0.02195), 0, -10, 0 },
	{ "ring_p_phalange",	 15,	vec3(-0.10867, -0.00686, -0.00121), 0, 0, 0 },
	{ "ring_m_phalange", 	 16,	vec3(-0.07301,  0.00107, -0.00119), 0, 0, 0 },
	{ "ring_d_phalange", 	 17,	vec3(-0.04452, -0.00198, -0.00237), 0, 0, 0 },
	{ "pinky_metacarpal",	 3,		vec3(-0.05420,  0.00761, -0.04451), 0, -17, 0 },
	{ "pinky_p_phalange",	 19,	vec3(-0.08685,  0.00310,  0.00080), 0, 0, 0 },
	{ "pinky_m_phalange",	 20,	vec3(-0.06265,  0.00052, -0.00187), 0, 0, 0 },
	{ "pinky_d_phalange",	 21,	vec3(-0.03819, -0.00212,  0.00148), 0, 0, 0 }
};

bone_info snitchbit_list[] = {
	{ "body", 				-1,		vec3(0, 0, 0), 0, 0, 0 },
	{ "leftWing", 			 0,		vec3(0, 0, 0), 0, 0, 0 },
	{ "rightWing", 			 0,		vec3(0, 0, 0), 0, 0, 0 }
};

std::string snitchTexes[] = {
	"Textures/bodytex.png",
	"Textures/wingtex.png",
	"Textures/wingtex.png"
};