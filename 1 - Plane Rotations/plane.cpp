#include "plane.h"

Plane::Plane() {
	q = quat_from_axis_deg(-0.0, 0.0f, 1.0f, 0.0f);
	move = vec3(0.0, 0.0, 0.0);
}

void Plane::setup(char* mesh_file_path, char* tex_file_path, char* prop_file_path, char* prop_tex_file_path, Shader shad) {
	plane_model = scene_object(mesh_file_path, tex_file_path);
	propellor = scene_object(prop_file_path, prop_tex_file_path);
	shader = shad;

	propellor.model = identity_mat4();
}

void Plane::yaw(float scalar) {
	euler_y += scalar;
}

void Plane::pitch(float scalar) {
	euler_x += scalar;
}

void Plane::roll(float scalar) {
	euler_z += scalar;
}

void Plane::q_yaw(float scalar) {
	q = quat_from_axis_deg(scalar, up.v[0], up.v[1], up.v[2])*q;
}

void Plane::q_pitch(float scalar) {
	q = quat_from_axis_deg(scalar, rgt.v[0], rgt.v[1], rgt.v[2])*q;
}
void Plane::q_roll(float scalar) {
	q = quat_from_axis_deg(scalar, fwd.v[0], fwd.v[1], fwd.v[2])*q;
}

void Plane::draw(){
	glUseProgram(shader.id);
	glUniformMatrix4fv(shader.V_loc, 1, GL_FALSE, identity_mat4().m);
	model_mat = translate(rotate_x_deg(rotate_y_deg(rotate_z_deg(identity_mat4(), euler_z), euler_y), euler_x), location);
	glUniformMatrix4fv(shader.M_loc, 1, GL_FALSE, model_mat.m);
	plane_model.draw();
	glUniformMatrix4fv(shader.M_loc, 1, GL_FALSE, ( model_mat*propellor.model).m);
	propellor.draw();

	propellor.model = prop_rot_mat*propellor.model;
}

void Plane::q_draw() {
	mat4 R = quat_to_mat4(q);
	fwd = R * vec4(0.0, 0.0, -1.0, 0.0);
	rgt = R * vec4(1.0, 0.0, 0.0, 0.0);
	up = R * vec4(0.0, 1.0, 0.0, 0.0);

	location = location + vec3(fwd) * -move.v[2];
	location = location + vec3(up) * move.v[1];
	location = location + vec3(rgt) * move.v[0];
	mat4 T = translate(identity_mat4(), vec3(location));
	move = vec3(0.0, 0.0, 0.0);
	model_mat = T*R;

	glUseProgram(shader.id);
	glUniformMatrix4fv(shader.V_loc, 1, GL_FALSE, identity_mat4().m);
	glUniformMatrix4fv(shader.M_loc, 1, GL_FALSE, model_mat.m);
	plane_model.draw();
	glUniformMatrix4fv(shader.M_loc, 1, GL_FALSE, (model_mat*propellor.model).m);
	propellor.draw();

	propellor.model = prop_rot_mat*propellor.model;
}

void Plane::fpp_draw() {
	model_mat = translate(identity_mat4(), vec3(0.0, -0.5, -0.3));

	glUseProgram(shader.id);
	glUniformMatrix4fv(shader.V_loc, 1, GL_FALSE, identity_mat4().m);
	//propellor first or you can't see it
	glUniformMatrix4fv(shader.M_loc, 1, GL_FALSE, (model_mat*propellor.model).m);
	propellor.draw();
	glUniformMatrix4fv(shader.M_loc, 1, GL_FALSE, model_mat.m);
	plane_model.draw();

	propellor.model = prop_rot_mat*propellor.model;

}

mat4 Plane::get_fpp_cube_view_mat() {
	mat4 R = quat_to_mat4(q);
	fwd = R * vec4(0.0, 0.0, -1.0, 0.0);
	rgt = R * vec4(1.0, 0.0, 0.0, 0.0);
	up = R * vec4(0.0, 1.0, 0.0, 0.0);

	return inverse(R);
}