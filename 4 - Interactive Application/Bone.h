#pragma once
#include "Antons_maths_funcs.h"
#include <GL/glew.h>
#include <vector>
#include <string>
#include "obj_parser.h"
#include "Grabbed.h"

class Bone {
public:
	std::string name;
	Bone *parent = NULL;
	std::vector<Bone*> children;
	GLuint vao;
	int point_count;
	GLuint tex_handle = NULL;
	
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

	void load_tex(const char* file_name) {
		load_texture(file_name, &tex_handle);
	}

	void set_offset(vec3 direction) {
		init_offset = direction;
	}

	void add_child(Bone* new_child) {
		children.push_back(new_child);
		new_child->parent = this;
	};
	
	void transform(mat4 parent_orientation) {
		local_transfomation = translate(local_orientation, init_offset);
		global_transformation = parent_orientation*local_transfomation;

		for (int i = 0; i < children.size(); i++) {
			children[i]->transform(global_transformation);
		}
	}

	void draw(GLuint M_loc) {
		if (tex_handle) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_handle);
		}

		glUniformMatrix4fv(M_loc, 1, GL_FALSE, global_transformation.m);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, point_count);
		for (int i = 0; i < children.size(); i++) {
			children[i]->draw(M_loc);
		}
	}

	vec3 world_coords() {
		return vec3(global_transformation.m[12], global_transformation.m[13], global_transformation.m[14]);
	}

	void grab(int grabCount) {
		if (grabCount <= 10) {
			if (grabMapping.find(name) != grabMapping.end()) {
				local_orientation = rotate_z_deg(local_orientation, grabMapping[name] / 10.0);
			}
		}
		else if (grabCount > 50) {
			if (grabMapping.find(name) != grabMapping.end()) {
				local_orientation = rotate_z_deg(local_orientation, -grabMapping[name] / 25.0);
			}
		}
		for (int i = 0; i < children.size(); i++) {
			children[i]->grab(grabCount);
		}
	}

};
