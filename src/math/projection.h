#ifndef MATH_PROJECTION_H
#define MATH_PROJECTION_H

typedef struct {
	float x;
	float y;
	float z;
} vec3;

typedef struct {
	float n[4][4];
} mat4;

const vec3 CAMERA_FRONT = vec3{ 0.0, 0.0, -1.0 };
const vec3 CAMERA_UP = vec3{ 0.0, 1.0, 0.0 };

mat4 projectionMatrix(float aspect, float fovY, float zNear, float zFar);
mat4 viewMatrix(const vec3 &eye, const vec3 &front);
mat4 modelMatrix(const vec3 &position);

#endif // !MATH_PROJECTION_H
