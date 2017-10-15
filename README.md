# realTimeAnimation
Collection of assignment projects for Real-Time Animation module for the Interactive Entertainment Technology Masters course.



Folders 1 - 4 contain source code directly related to the real-time physics problems indicated by the folder names only.

The folder labelled "Third Party Files" contains various files that I have not written but which I have used to implement functionality such as writing text to screen, or vector and matrix mathematics.

Project Descriptions:


1 - Plane Rotations

An implementation of rotation on an airplane model using both Euler angles and quaternions. Also implements a first-person view when using quaternions.

2 - Hand Heirarchy

Implements hierarchical animation on a skeletal hand model.

3 - Arm Inverse Kinematics

Implements inverse kinematics for the motion of an arm using Cyclic Coordinate Descent. The position of the end effector can be specified using the mouse or the hand can be made to follow a curve described by a Catmull-Rom Spline with four anchor points.

4 - Interactive Application

This is essentially a small game in which you control the arm of a skeleton and attempt to grab a ﬂying ball (modelled after the golden snitch from the Harry Potter series) as it ﬂits about in the air.

External Libraries used:

• Antons_maths_funcs: a library for vector, matrix and quaternion mathematics.

• gl_utils: (slightly modiﬁed), a library of common OpenGL functionality.

• text: a library for writing text on-screen in an OpengGL application.

• obj_parser: to provide mesh loading functionality.

• stb_image: an image loading library.

• assimp: for loading 3D meshes.

• glew: the OpenGL extension wrangler.

• glfw: an OpenGL development framework.