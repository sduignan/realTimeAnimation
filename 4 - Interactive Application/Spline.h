#pragma once
#include "Antons_maths_funcs.h"


class Spline {
public:
	vec3* P;
	vec3 coefficients[4];
	Spline() {};
	Spline(vec3* control_points) {
		P = control_points;
		coefficients[0] = P[1] * 2;
		coefficients[1] = (P[0] * -1) + P[2];
		coefficients[2] = (P[0] * 2) - (P[1] * 5) + (P[2] * 4) - P[3];
		coefficients[3] = (P[0] * -1) + (P[1] * 3) - (P[2] * 3) + P[3];
	};

	void setPoints(vec3* control_points) {
		P = control_points;
		coefficients[0] = P[1] * 2;
		coefficients[1] = (P[0] * -1) + P[2];
		coefficients[2] = (P[0] * 2) - (P[1] * 5) + (P[2] * 4) - P[3];
		coefficients[3] = (P[0] * -1) + (P[1] * 3) - (P[2] * 3) + P[3];
	};

	vec3 pointAtT(float t) {
		float t2 = t*t;
		return (coefficients[0] + coefficients[1]*t + coefficients[2]*t2 + coefficients[3]*(t2*t))*0.5;
	};
};
