#ifndef MATH_PROJECTION_H
#define MATH_PROJECTION_H

#include "types/mat4.h"
#include "types/vec3.h"

namespace math {

mat4 perspective(float aspect, float fovY, float zNear, float zFar);
mat4 lookAt(const vec3 &eye, const vec3 &target, const vec3 &up);

}; // namespace math

#endif // !MATH_PROJECTION_H
