#pragma once
#include "Antons_maths_funcs.h"
#include <GL/glew.h>
#include <vector>
#include <string>
#include "obj_parser.h"

class Bone {
public:
	std::string name;
	Bone *parent = NULL;
	std::vector<Bone*> children;
	GLuint vao;
	int point_count;
	float rot = 0;

	vec3 rotation_axis;

	vec3 init_offset;
	mat4 local_orientation;
	mat4 local_transfomation;
	mat4 global_transformation;

	Bone() {
		local_orientation = translate(identity_mat4(), vec3(0, 0.05, 0));
		local_transfomation = identity_mat4();
	};

	~Bone() {};

	void load_mesh(const char* file_name) {
		load_mesh_to_vao(file_name, point_count, vao);
	}

	void set_offset(vec3 direction) {
		init_offset = direction;
	}

	void add_child(Bone* new_child) {
		children.push_back(new_child);
		new_child->parent = this;
	};

	void set_axis() {
		if (parent) {
			rotation_axis = normalise(cross(init_offset, vec3(0, 0, 1)));
		}
		else {
			rotation_axis = vec3(1, 0, 0);
		}
	}

	void transform(mat4 parent_orientation) {
		local_transfomation = translate(local_orientation, init_offset);
		global_transformation = parent_orientation*local_transfomation;

		for (int i = 0; i < children.size(); i++) {
			children[i]->transform(global_transformation);
		}
	}

	void draw(GLuint M_loc) {
		glUniformMatrix4fv(M_loc, 1, GL_FALSE, global_transformation.m);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, point_count);
		for (int i = 0; i < children.size(); i++) {
			children[i]->draw(M_loc);
		}
	}

	void print() {
		printf("Name: %s\tRotation: %f\n", name.c_str(), rot);
		for (int i = 0; i < children.size(); i++) {
			children[i]->print();
		}
	}
};