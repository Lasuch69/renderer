#ifndef MATH_PROJECTION_H
#define MATH_PROJECTION_H

namespace math {

struct mat4;
struct vec3;

mat4 perspective(float aspect, float fov_y, float z_near, float z_far);
mat4 lookAt(const vec3 &eye, const vec3 &target, const vec3 &up);

}; // namespace math

#endif // !MATH_PROJECTION_H
