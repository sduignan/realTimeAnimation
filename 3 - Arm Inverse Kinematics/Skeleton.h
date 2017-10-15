#pragma once
#include "Bone.h"

class Skeleton {
public:
	Bone *root;
	int bone_count = 0;
	mat4 root_transform;

	Skeleton() {
		root = NULL;
		root_transform = identity_mat4();
	};

	void transform(mat4 root_transformation) {
		root_transform = root_transformation*root_transform;
		root->transform(root_transform);
	};

	void draw(GLuint M_loc) {
		root->draw(M_loc);
	}
};