#pragma once
#include "Antons_maths_funcs.h"
#include "Bone.h"

#define CCD_THRESHHOLD 0.000001
#define CCD_ITERATIONS 200

bool ComputeCCDLink(vec3 targetPosition, Bone* currBone, Bone* endEffector) {
	vec3 currBoneWorldPos = currBone->world_coords();
	vec3 endBoneWorldPos = endEffector->world_coords();

	// Check if end effector is close enough
	if (length2(targetPosition - endBoneWorldPos) > CCD_THRESHHOLD) {
		// Vector to current end effector position
		vec3 currVector = normalise(endBoneWorldPos - currBoneWorldPos);
		// Vector to desired end efector position
		vec3 targetVector = normalise(targetPosition - currBoneWorldPos);

		float cosAngle = dot(targetVector, currVector);

		if (cosAngle < 0.999999 && cosAngle > -0.999999) {
			vec3 crossProd = cross(targetVector, currVector);

			if (length2(crossProd)<CCD_THRESHHOLD) {
				return true;
			}
			
			crossProd = normalise(crossProd);

			// Radians
			float turnAngle = acos(cosAngle);

			versor quat = quat_from_axis_deg(-turnAngle, crossProd);

			currBone->local_orientation = quat_to_mat4(quat) * currBone->local_orientation;
			return false;
		}
		else {
			return true;
		}
	}
	else {
		return true;
	}
}

bool ComputeCCDZLink(vec3 targetPosition, Bone* currBone, Bone* endEffector) {
	vec3 currBoneWorldPos = currBone->world_coords();
	vec3 endBoneWorldPos = endEffector->world_coords();
	targetPosition.v[2] = endBoneWorldPos.v[2];
	currBoneWorldPos.v[2] = endBoneWorldPos.v[2];
	
	if (length2(targetPosition - endBoneWorldPos) > CCD_THRESHHOLD) {
		// Vector to current end effector position
		vec3 currVector = normalise(endBoneWorldPos - currBoneWorldPos);
		// Vector to desired end efector position
		vec3 targetVector = normalise(targetPosition - currBoneWorldPos);

		float cosAngle = dot(targetVector, currVector);

		if (cosAngle < 0.999999) {
			vec3 crossProd = cross(targetVector, currVector);

			// Radians
			float turnAngle = acos(cosAngle);
			float turnDeg = turnAngle*ONE_DEG_IN_RAD;
			
			// DAMPING WOULD GO HERE

			float multiplier = 1.0;
			// Z positive -> Rotate Clockwise
			if (crossProd.v[2] > 0.0) {
				multiplier = -1.0;
			}
			
			currBone->local_orientation = rotate_z_deg(currBone->local_orientation, multiplier*turnDeg);
			return false;
		}
		else {
			return true;
		}
	}
	else {
		return true;
	}
}

void CCD(Skeleton theBoneMan, vec3 targetPosition, Bone* endEffector, std::string tipTopBoneName) {
	bool done = false;
	int tries = 0;

	while (!done && tries <= CCD_ITERATIONS) {
		Bone* currBone = endEffector->parent;
		done = true;
		while (currBone->name != tipTopBoneName) {
			bool tempDone = ComputeCCDLink(targetPosition, currBone, endEffector);
			done = done && tempDone;
			currBone = currBone->parent;
		}
		if (currBone->parent != NULL) {
			currBone->transform(currBone->parent->global_transformation);
		}
		else {
			currBone->transform(identity_mat4());
		}
		tries++;
	}
}