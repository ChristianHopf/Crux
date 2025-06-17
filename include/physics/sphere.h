#pragma once

#include <cglm/cglm.h>

struct Sphere {
  vec3 center;
  float radius;
};

void sphere_intersect_aabb();
void sphere_intersect_plane();
void sphere_intersect_sphere();
